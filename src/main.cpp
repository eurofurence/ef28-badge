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
#include <EFTouch.h>

#include "FSM.h"
#include "FSMGlobals.h"
#include "util.h"

// Global objects and states
constexpr unsigned int INTERVAL_BATTERY_CHECK = 10000;
// Initializing the board with a brightness above 48 can cause stability issues!
constexpr uint8_t ABSOLUTE_MAX_BRIGHTNESS = 45;
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
    unsigned char allShortpress:         1;
    unsigned char allLongpress:          1;
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
void ARDUINO_ISR_ATTR isr_allShortpress()         { isrEvents.allShortpress = 1; }
void ARDUINO_ISR_ATTR isr_allLongpress()          { isrEvents.allLongpress = 1; }

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
    EFLed.setBrightnessPercent(30);
    EFLed.clear();
    EFLed.setDragonNose(CRGB::Red);

    // Hard brown out can only be cleared by board reset
    while (1) {
        // Low brightness blink every few seconds
        EFLed.enablePower();
        EFLed.setDragonNose(CRGB::Red);
        esp_sleep_enable_timer_wakeup(100 * 1000);  // 100 ms
        EFLed.disablePower();
        // sleep most of the time.
        esp_sleep_enable_timer_wakeup(2 * 1000 * 1000);  // 2s
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
    EFLed.setBrightnessPercent(40);

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

void batteryCheck() {
    EFBoardPowerState previousState = pwrstate;
    pwrstate = EFBoard.updatePowerState();
    if (previousState != pwrstate) {
        LOGF_DEBUG("Updated power state: %s\r\n", toString(pwrstate));
    }

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
}

/**
 * @brief Calculates a wave animation. Used by bootupAnimation()
 */
float wave_function(float x, float start, float end, float amplitude) {
    if (x < start || x > end) {
        return 0;
    }
    double normalized_x = (x - start) / (end - start) * M_PI;
    return amplitude * std::sin(normalized_x);
}

/**
 * @brief Displays a fancy bootup animation
 */
void boopupAnimation() {
    CRGB data[EFLED_TOTAL_NUM];
    fill_solid(data, EFLED_TOTAL_NUM, CRGB::Black);
    EFLed.setAll(data);
    delay(100);

    // Origin point. Power-Button is 11, 25. Make it originate from where the hand is
    constexpr int16_t pwrX = -30;
    constexpr int16_t pwrY = 16;
    uint8_t hue = 120;  // Green

    for (uint16_t n = 0; n < 30; n++) {
        uint16_t n_scaled = n * 7;
        if (n % 10) {
            // Low batteries might crash the boopup animation
            batteryCheck();
        }
        for (uint8_t i = 0; i < EFLED_TOTAL_NUM; i++) {
            int16_t dx = EFLedClass::getLEDPosition(i).x - pwrX;
            int16_t dy = EFLedClass::getLEDPosition(i).y - pwrY;
            float distance = sqrt(dx * dx + dy * dy);

            float intensity = wave_function(distance, n_scaled / 2 - 30, n_scaled * 2 + 20, 1.0);
            intensity = intensity * intensity; // sharpen wave

            // energy front
            uint8_t value = static_cast<uint8_t>(intensity * 255);
            data[i] = CHSV((hue + static_cast<uint16_t>(distance * 2.0f)) % 255, 240, value);
        }
        EFLed.setAll(data);
        delay(15);
    }
    EFLed.clear();
    delay(400);

    batteryCheck();
    // dragon awakens ;-)
    EFLed.setDragonEye(CRGB(10,0, 0));
    delay(60);
    EFLed.setDragonEye(CRGB(50,0, 0));
    delay(80);
    EFLed.setDragonEye(CRGB(100,0, 0));
    delay(150);
    EFLed.setDragonEye(CRGB(200,0, 0));
    delay(700);
    EFLed.setDragonEye(CRGB(100,0, 0));
    delay(80);
    EFLed.setDragonEye(CRGB(50,0, 0));
    delay(80);
    EFLed.setDragonEye(CRGB(10,0, 0));
    delay(60);
    EFLed.setDragonNose(CRGB::Black);
    delay(200);
}

/**
 * @brief Initial board setup. Called at boot / board reset.
 */
void setup() {
    // Init board
    EFBoard.setup();
    EFLed.init(ABSOLUTE_MAX_BRIGHTNESS);
    EFLed.setBrightnessPercent(40);  // We do not have access to the settings yet, default to 40
    boopupAnimation();
    
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
    EFTouch.attachInterruptOnShortpress(EFTouchZone::All, isr_allShortpress);
    EFTouch.attachInterruptOnLongpress(EFTouchZone::All, isr_allLongpress);

    // Get FSM going
    fsm.resume();
}

/**
 * @brief Main program loop
 */
void loop() {
    // Handler: ISR Events
    if (isrEvents.allLongpress) {
        fsm.queueEvent(FSMEvent::AllLongpress);
        isrEvents.noseLongpress = false;
        isrEvents.noseShortpress = false;
        isrEvents.noseRelease = false;
        isrEvents.fingerprintLongpress = false;
        isrEvents.fingerprintShortpress = false;
        isrEvents.fingerprintRelease = false;
        isrEvents.allLongpress = false;
        isrEvents.allShortpress = false;
    }
    if (isrEvents.allShortpress) {
        fsm.queueEvent(FSMEvent::AllShortpress);
        isrEvents.noseShortpress = false;
        isrEvents.noseRelease = false;
        isrEvents.fingerprintShortpress = false;
        isrEvents.fingerprintRelease = false;
        isrEvents.allShortpress = false;
    }
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
        batteryCheck();
        task_battery = millis() + INTERVAL_BATTERY_CHECK;
    }
}
