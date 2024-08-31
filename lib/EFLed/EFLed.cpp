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
#include <FastLED.h>

#include <EFLogging.h>

#include "EFLed.h"

EFLedClass::EFLedClass()
: max_brightness(0)
, led_data({0})
{
}

void EFLedClass::init() {
    this->init(EFLED_MAX_BRIGHTNESS_DEFAULT);
}

void EFLedClass::init(const uint8_t max_brightness) {
    for (uint8_t i = 0; i < EFLED_TOTAL_NUM; i++) {
        this->led_data[i] = CRGB::Black;
    }
    LOG_INFO("(EFLed) Initialized internal LED data struct");

    FastLED.clearData();
    FastLED.addLeds<WS2812B, EFLED_PIN_LED_DATA, GRB>(this->led_data, EFLED_TOTAL_NUM);
    LOGF_DEBUG("(EFLed) Added new WS2812B: %d LEDs @ PIN %d\r\n", EFLED_TOTAL_NUM, EFLED_PIN_LED_DATA);

    this->max_brightness = max_brightness;
    FastLED.setBrightness(this->max_brightness);
    LOGF_DEBUG("(EFLed) Set max_brightness=%d\r\n", this->max_brightness)

    enablePower();

}

void EFLedClass::enablePower() {
    pinMode(EFLED_PIN_5VBOOST_ENABLE, OUTPUT);
    digitalWrite(EFLED_PIN_5VBOOST_ENABLE, HIGH);
    LOG_INFO("(EFLed) Enabled +5V boost converter");
    delay(10);
}

void EFLedClass::disablePower() {
    digitalWrite(EFLED_PIN_5VBOOST_ENABLE, LOW);
    LOG_INFO("(EFLed) Disabled +5V boost converter");
    delay(10);
}

void EFLedClass::clear() {
    for (uint8_t i = 0; i < EFLED_TOTAL_NUM; i++) {
        this->led_data[i] = CRGB::Black;
    }
    FastLED.show();
}

void EFLedClass::setBrightness(uint8_t brightness) const {
    FastLED.setBrightness(round((min(brightness, (uint8_t) 100) / (float) 100) * this->max_brightness));
    FastLED.show();
}

uint8_t EFLedClass::getBrightness() const {
    return (uint8_t) round(FastLED.getBrightness() / (float) this->max_brightness * 100);
}

void EFLedClass::setAll(const CRGB color[EFLED_TOTAL_NUM]) {
    for (uint8_t i = 0; i < EFLED_TOTAL_NUM; i++) {
        this->led_data[i] = color[i];
    }
    FastLED.show();
}

void EFLedClass::setAllSolid(const CRGB color) {
    for (uint8_t i = 0; i < EFLED_TOTAL_NUM; i++) {
        this->led_data[i] = color;
    }
    FastLED.show();
}

void EFLedClass::setDragonNose(const CRGB color) {
    this->led_data[EFLED_DRAGON_NOSE_IDX] = color;
    FastLED.show();
}

void EFLedClass::setDragonMuzzle(const CRGB color) {
    this->led_data[EFLED_DRAGON_MUZZLE_IDX] = color;
    FastLED.show();
}

void EFLedClass::setDragonEye(const CRGB color) {
    this->led_data[EFLED_DRAGON_EYE_IDX] = color;
    FastLED.show();
}

void EFLedClass::setDragonCheek(const CRGB color) {
    this->led_data[EFLED_DRAGON_CHEEK_IDX] = color;
    FastLED.show();
}

void EFLedClass::setDragonEarBottom(const CRGB color) {
    this->led_data[EFLED_DRAGON_EAR_BOTTOM_IDX] = color;
    FastLED.show();
}

void EFLedClass::setDragonEarTop(const CRGB color) {
    this->led_data[EFLED_DRAGON_EAR_TOP_IDX] = color;
    FastLED.show();
}

void EFLedClass::setDragon(const CRGB color[EFLED_DRAGON_NUM]) {
    for (uint8_t i = 0; i < EFLED_DRAGON_NUM; i++) {
        this->led_data[EFLED_DARGON_OFFSET + i] = color[i];
    }
    FastLED.show();
}

void EFLedClass::setEFBar(const CRGB color[EFLED_EFBAR_NUM]) {
    for (uint8_t i = 0; i < EFLED_EFBAR_NUM; i++) {
        this->led_data[EFLED_EFBAR_OFFSET + i] = color[i];
    }
    FastLED.show();
}

void EFLedClass::setEFBar(uint8_t idx, const CRGB color) {
    if (idx >= EFLED_EFBAR_NUM) {
        return;
    }

    this->led_data[EFLED_EFBAR_OFFSET + idx] = color;
    FastLED.show();
}

void EFLedClass::setEFBarCursor(
    uint8_t idx,
    const CRGB color_on,
    const CRGB color_off
) {
    for (uint8_t i = 0; i < EFLED_EFBAR_NUM; i++) {
        this->led_data[EFLED_EFBAR_OFFSET + i] = (i == idx) ? color_on : color_off;
    }
    FastLED.show();
}

void EFLedClass::fillEFBarProportionally(
    uint8_t percent,
    const CRGB color_on,
    const CRGB color_off
) {
    uint8_t num_leds_on = min(int(round(percent / (100.0f / EFLED_EFBAR_NUM))), EFLED_EFBAR_NUM);
    for (uint8_t i = 0; i < num_leds_on; i++) {
        this->led_data[EFLED_EFBAR_OFFSET + i] = color_on;
    }
    for (uint8_t i = num_leds_on; i < EFLED_EFBAR_NUM; i++) {
        this->led_data[EFLED_EFBAR_OFFSET + i] = color_off;
    }
    FastLED.show();
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EFLED)
EFLedClass EFLed;
#endif
