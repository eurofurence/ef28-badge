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
#include "secrets.h"


bool blinkled_state = false;
uint8_t flagidx = 0;
uint32_t brightness = 0;

unsigned long task_blinkled = 0;
unsigned long task_flagswitch = 0;
unsigned long task_touchleds = 0;
unsigned long task_brightness = 0;


volatile bool logtouched = false;
void ARDUINO_ISR_ATTR isr_touched() {logtouched = true;}

volatile bool loguntouched = false;
void ARDUINO_ISR_ATTR isr_untouched() {loguntouched=true;}

volatile bool logshortpress = false;
void ARDUINO_ISR_ATTR isr_shortpressed() {logshortpress=true; }

volatile bool loglongpress = false;
void ARDUINO_ISR_ATTR isr_longpressed() {loglongpress=true; }


void setup() {
    EFBoard.setup();
    EFLed.init(20);

    EFLed.setDragonNose(CRGB::Red);
    if (EFBoard.connectToWifi(WIFI_SSID, WIFI_PASSWORD)) {
        EFLed.setDragonNose(CRGB::Green);
    }

    EFBoard.enableOTA(OTA_SECRET);
    EFLed.setDragonMuzzle(CRGB::Green);
    
    // TODO: Just testing, remove me later
    delay(2000);
    EFBoard.disableOTA();
    EFBoard.disableWifi();
    delay(1000);
    EFBoard.connectToWifi(WIFI_SSID, WIFI_PASSWORD);
    EFBoard.enableOTA(OTA_SECRET);

    // Touchy stuffy
    EFTouch.init();
    EFTouch.attachInterruptOnTouch(EFTouchZone::Fingerprint, isr_touched);
    EFTouch.attachInterruptOnRelease(EFTouchZone::Fingerprint, isr_untouched);
    EFTouch.attachInterruptOnShortpress(EFTouchZone::Fingerprint, isr_shortpressed);
    EFTouch.attachInterruptOnLongpress(EFTouchZone::Fingerprint, isr_longpressed);
}

void loop() {
    // Handler: OTA
    ArduinoOTA.handle();

    // Task: Blink LED
    if (task_blinkled < millis()) {
        EFLed.setDragonEye(blinkled_state ? CRGB::Green : CRGB::Black);
        blinkled_state = !blinkled_state;

        task_blinkled = millis() + 1000;
    }

    // Task: Switch pride flag
    if (task_flagswitch < millis()) {
        LOG_DEBUG("Switched pride flag");

        switch (flagidx) {
            case 0: EFLed.setEFBar(EFPrideFlags::LGBT); break;
            case 1: EFLed.setEFBar(EFPrideFlags::LGBTQI); break;
            case 2: EFLed.setEFBar(EFPrideFlags::Bisexual); break;
            case 3: EFLed.setEFBar(EFPrideFlags::Polyamorous); break;
            case 4: EFLed.setEFBar(EFPrideFlags::Polysexual); break;
            case 5: EFLed.setEFBar(EFPrideFlags::Transgender); break;
            case 6: EFLed.setEFBar(EFPrideFlags::Pansexual); break;
            case 7: EFLed.setEFBar(EFPrideFlags::Asexual); break;
            case 8: EFLed.setEFBar(EFPrideFlags::Genderfluid); break;
            case 9: EFLed.setEFBar(EFPrideFlags::Genderqueer); break;
            case 10: EFLed.setEFBar(EFPrideFlags::Nonbinary); break;
            case 11: EFLed.setEFBar(EFPrideFlags::Intersex); break;
        }
        flagidx = (flagidx + 1) % 12;

        task_flagswitch = millis() + 3000;
    }

    // Task: Touch LEDs
    if (task_touchleds < millis()) {
        uint8_t touchy = EFTouch.readFingerprint();
        if (touchy) {
            EFLed.fillEFBarProportionally(touchy*10, CRGB::Purple, CRGB::Black);
        }

        task_touchleds = millis() + 10;
    }

    // Task: Brightness
    if (task_brightness < millis()) {
        uint8_t newbrightness = (brightness % 202) < 101 ? brightness % 101 : 100 - (brightness % 101);
        EFLed.setBrightness(newbrightness);
        brightness++;

        task_brightness = millis() + 10;
    }

    // Handler: ISR debug logging
    if (logtouched) {
        logtouched = false;
        LOG_DEBUG("v TOUCHED!");
    }
    if (logshortpress) {
        logshortpress = false;
        LOG_DEBUG(" - shortpress");
    }
    if (loglongpress) {
        loglongpress = false;
        LOG_DEBUG(" ----- longpress");
    }
    if (loguntouched) {
        loguntouched = false;
        LOG_DEBUG("^ released.");
    }

    // if (deepsleepcounter == 0) {
    //     USBSerial.println("Putting the ESP into deep sleep for 3 seconds ...");
    //     esp_sleep_enable_timer_wakeup(3000000);
    // 
    //     USBSerial.flush();
    //     esp_deep_sleep_start();
    // }
}
