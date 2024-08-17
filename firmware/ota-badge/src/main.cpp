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

#include "constants.h"
#include "FSM.h"
#include "secrets.h"

FSM fsm(10);

bool blinkled_state = false;
unsigned long task_fsm_handle = 0;
unsigned long task_blinkled = 0;
unsigned long task_battery = 0;

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

void ARDUINO_ISR_ATTR isr_fingerprintTouch()      { isrEvents.fingerprintTouch = 1; }
void ARDUINO_ISR_ATTR isr_fingerprintRelease()    { isrEvents.fingerprintRelease = 1; }
void ARDUINO_ISR_ATTR isr_fingerprintShortpress() { isrEvents.fingerprintShortpress = 1; }
void ARDUINO_ISR_ATTR isr_fingerprintLongpress()  { isrEvents.fingerprintLongpress = 1; }
void ARDUINO_ISR_ATTR isr_noseTouch()             { isrEvents.noseTouch = 1; }
void ARDUINO_ISR_ATTR isr_noseRelease()           { isrEvents.noseRelease = 1; }
void ARDUINO_ISR_ATTR isr_noseShortpress()        { isrEvents.noseShortpress = 1; }
void ARDUINO_ISR_ATTR isr_noseLongpress()         { isrEvents.noseLongpress = 1; }

void setup() {
    // Init board
    EFBoard.setup();
    EFLed.init(16);
    
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
}

void loop() {
    // Handler: ISR Events
    if (isrEvents.fingerprintTouch) {
        fsm.queueEvent(FSMEvent::FingerprintTouch);
        isrEvents.fingerprintTouch = false;
    }
    if (isrEvents.fingerprintRelease) {
        fsm.queueEvent(FSMEvent::FingerprintRelease);
        isrEvents.fingerprintRelease = false;
    }
    if (isrEvents.fingerprintShortpress) {
        fsm.queueEvent(FSMEvent::FingerprintShortpress);
        isrEvents.fingerprintShortpress = false;
    }
    if (isrEvents.fingerprintLongpress) {
        fsm.queueEvent(FSMEvent::FingerprintLongpress);
        isrEvents.fingerprintLongpress = false;
    }
    if (isrEvents.noseTouch) {
        fsm.queueEvent(FSMEvent::NoseTouch);
        isrEvents.noseTouch = false;
    }
    if (isrEvents.noseRelease) {
        fsm.queueEvent(FSMEvent::NoseRelease);
        isrEvents.noseRelease = false;
    }
    if (isrEvents.noseShortpress) {
        fsm.queueEvent(FSMEvent::NoseShortpress);
        isrEvents.noseShortpress = false;
    }
    if (isrEvents.noseLongpress) {
        fsm.queueEvent(FSMEvent::NoseLongpress);
        isrEvents.noseLongpress = false;
    }

    // Task: Handle FSM
    if (task_fsm_handle < millis()) {
        fsm.handle();
        task_fsm_handle = millis() + fsm.getTickRateMs();
    }

    // Task: Blink LED
    if (task_blinkled < millis()) {
        EFLed.setDragonEarTop(blinkled_state ? CRGB::Green : CRGB::Black);
        blinkled_state = !blinkled_state;

        task_blinkled = millis() + 1000;
    }

    // Task: Battery voltage
    if (task_battery < millis()) {
        if (EFBoard.isBatteryPowered()) {
            LOGF_DEBUG(
                "Battery voltage: %.2f V (%d %%)\r\n",
                EFBoard.getBatteryVoltage(),
                EFBoard.getBatteryCapacityPercent()
            );
        }

        task_battery = millis() + 60000;
    }
}
