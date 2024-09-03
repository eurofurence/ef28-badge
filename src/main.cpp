// MIT License
//
// Copyright 2024 Eurofurence e.V. 
// 
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the “Software”),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

/**
 * @author Honigeintopf
 */

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#include <WiFi.h>

#include <EFBoard.h>
#include <EFLogging.h>
#include <EFLed.h>
#include <EFPrideFlags.h>
#include <EFTouch.h>

#include "FSM.h"
#include "secrets.h"
#include "util.h"

// Global objects and states
constexpr unsigned int INTERVAL_BATTERY_CHECK = 60000;
FSM fsm(10);
EFBoardPowerState pwrstate;

// Task counters
unsigned long task_fsm_handle = 0;
unsigned long task_blinkled = 0;
unsigned long task_battery = 0;
unsigned long task_brownout = 0;

/**
 * @brief Struct for interrupt event tracking / handling
 */
volatile struct ISREventsType {
    unsigned char fingerprintTouch:      1;
    unsigned char fingerprintRelease:    1;
    unsigned char fingerprintShortpress: 1;
    unsigned char fingerprintLongpress:  1;
    unsigned char noseTouch:             1;
    unsigned char noseRelease:           1;
    unsigned char noseShortpress:        1;
    unsigned char noseLongpress:         1;
} isrEvents;

// Interrupt service routines to update ISR struct upon triggering
void ARDUINO_ISR_ATTR isr_fingerprintTouch()      { isrEvents.fingerprintTouch = 1; }
void ARDUINO_ISR_ATTR isr_fingerprintRelease()    { isrEvents.fingerprintRelease = 1; }
void ARDUINO_ISR_ATTR isr_fingerprintShortpress() { isrEvents.fingerprintShortpress = 1; }
void ARDUINO_ISR_ATTR isr_fingerprintLongpress()  { isrEvents.fingerprintLongpress = 1; }
void ARDUINO_ISR_ATTR isr_noseTouch()             { isrEvents.noseTouch = 1; }
void ARDUINO_ISR_ATTR isr_noseRelease()           { isrEvents.noseRelease = 1; }
void ARDUINO_ISR_ATTR isr_noseShortpress()        { isrEvents.noseShortpress = 1; }
void ARDUINO_ISR_ATTR isr_noseLongpress()         { isrEvents.noseLongpress = 1; }

/**
 * @brief Handles hard brown out events
 */
void _hardBrownOutHandler() {
    // Entry actions
    LOGF_ERROR(
        "HARD BROWN OUT DETECTED (V_BAT = %.2f V). Panic!\r\n",
        EFBoard.getBatteryVoltage()
    );
    EFBoard.disableWifi();
    // Try getting the LEDs into some known state
    EFLed.setBrightnessPercent(20);
    EFLed.clear();
    EFLed.setDragonNose(CRGB::Red);

    // Hard brown out can only be cleared by board reset
    while (1) {
        // Low brightness blink every few seconds
        EFLed.enablePower();
        EFLed.setDragonNose(CRGB::Red);
        esp_sleep_enable_timer_wakeup(80 * 1000);  // in ms
        EFLed.disablePower();
        // sleep most of the time.
        esp_sleep_enable_timer_wakeup(4 * 1000000);
        esp_light_sleep_start();
        // TODO: Go to deep sleep, but save the reason so when waking up, we can blink and deep sleep again
    }
}

/**
 * @brief Handles soft brown out events
 */
void _softBrownOutHandler() {
    // Entry actions
    LOGF_WARNING(
        "Soft brown out detected (V_BAT = %.2f V). Aborting main loop and display warning LED.\r\n",
        EFBoard.getBatteryVoltage()
    );
    EFBoard.disableWifi();
    EFLed.clear();
    EFLed.enablePower();
    EFLed.setBrightnessPercent(50);

    // Soft brown out can only be cleared by board reset but can escalate to hard brown out
    while (1) {
        // Check for hard brown out
        pwrstate = EFBoard.updatePowerState();
        if (pwrstate == EFBoardPowerState::BAT_BROWN_OUT_HARD) {
            _hardBrownOutHandler();
        }

        // Blink LED to signal brown out to user
        for (uint8_t n = 0; n < 30; n++) {
            EFLed.enablePower();
            EFLed.setDragonNose(CRGB::Red);
            esp_sleep_enable_timer_wakeup(200 * 1000);
            esp_light_sleep_start();
            EFLed.disablePower();
            esp_sleep_enable_timer_wakeup(800 * 1000);
            esp_light_sleep_start();
        }

        // Log current battery level
        LOGF_INFO(
            "Battery voltage: %.2f V (%d %%)\r\n",
            EFBoard.getBatteryVoltage(),
            EFBoard.getBatteryCapacityPercent()
        );
    }
}

/**
 * @brief Initial board setup. Called at boot / board reset.
 */
void setup() {
    // Init board
    EFBoard.setup();
    EFLed.init(30);
    
    // Touchy stuff
    EFTouch.init();
    EFTouch.attachInterruptOnTouch(EFTouchZone::Fingerprint, isr_fingerprintTouch);
    EFTouch.attachInterruptOnRelease(EFTouchZone::Fingerprint, isr_fingerprintRelease);
    EFTouch.attachInterruptOnShortpress(EFTouchZone::Fingerprint, isr_fingerprintShortpress);
    EFTouch.attachInterruptOnLongpress(EFTouchZone::Fingerprint, isr_fingerprintLongpress);
    EFTouch.attachInterruptOnTouch(EFTouchZone::Nose, isr_noseTouch);
    EFTouch.attachInterruptOnRelease(EFTouchZone::Nose, isr_noseRelease);
    EFTouch.attachInterruptOnShortpress(EFTouchZone::Nose, isr_noseShortpress);
    EFTouch.attachInterruptOnLongpress(EFTouchZone::Nose, isr_noseLongpress);
    
    // Get FSM going
    fsm.resume();
}

/**
 * @brief Main program loop
 */
void loop() {
    // Handler: ISR Events
    if (isrEvents.fingerprintTouch) {
        fsm.queueEvent(FSMEvent::FingerprintTouch);
        isrEvents.fingerprintTouch = false;
    }
    if (isrEvents.fingerprintLongpress) {
        fsm.queueEvent(FSMEvent::FingerprintLongpress);
        isrEvents.fingerprintLongpress = false;
        isrEvents.fingerprintShortpress = false;
        isrEvents.fingerprintRelease = false;
    }
    if (isrEvents.fingerprintShortpress) {
        fsm.queueEvent(FSMEvent::FingerprintShortpress);
        isrEvents.fingerprintShortpress = false;
        isrEvents.fingerprintRelease = false;
    }
    if (isrEvents.fingerprintRelease) {
        fsm.queueEvent(FSMEvent::FingerprintRelease);
        isrEvents.fingerprintRelease = false;
    }

    if (isrEvents.noseTouch) {
        fsm.queueEvent(FSMEvent::NoseTouch);
        isrEvents.noseTouch = false;
    }
    if (isrEvents.noseLongpress) {
        fsm.queueEvent(FSMEvent::NoseLongpress);
        isrEvents.noseLongpress = false;
        isrEvents.noseShortpress = false;
        isrEvents.noseRelease = false;
    }
    if (isrEvents.noseShortpress) {
        fsm.queueEvent(FSMEvent::NoseShortpress);
        isrEvents.noseShortpress = false;
        isrEvents.noseRelease = false;
    }
    if (isrEvents.noseRelease) {
        fsm.queueEvent(FSMEvent::NoseRelease);
        isrEvents.noseRelease = false;
    }

    // Task: Handle FSM
    if (task_fsm_handle < millis()) {
        fsm.handle();
        task_fsm_handle = millis() + fsm.getTickRateMs();
    }

    // Task: Battery checks
    if (task_battery < millis()) {
        pwrstate = EFBoard.updatePowerState();
        LOGF_DEBUG("Updated power state: %s\r\n", toString(pwrstate));

        // Log battery level if battery powered
        if (EFBoard.isBatteryPowered()) {
            LOGF_INFO(
                "Battery voltage: %.2f V (%d %%)\r\n",
                EFBoard.getBatteryVoltage(),
                EFBoard.getBatteryCapacityPercent()
            );
        }
        
        // Handle brown out
        if (pwrstate == EFBoardPowerState::BAT_BROWN_OUT_HARD) {
            _hardBrownOutHandler();
        } else if (pwrstate == EFBoardPowerState::BAT_BROWN_OUT_SOFT) {
            _softBrownOutHandler();
        }

        task_battery = millis() + INTERVAL_BATTERY_CHECK;
    }
}
