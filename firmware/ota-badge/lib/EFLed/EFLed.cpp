#include <Arduino.h>
#include <FastLED.h>

#include "EFLed.h"

CRGB EFLed::led_data[EFLED_TOTAL_NUM];

EFLed::EFLed(): EFLed(EFLED_MAX_BRIGHTNESS_DEFAULT) {}

EFLed::EFLed(uint8_t max_brightness) {
    this->max_brightness = max_brightness;
    
    for (uint8_t i = 0; i < EFLED_TOTAL_NUM; i++) {
        this->led_data[i] = CRGB::Black;
    }

    FastLED.clearData();
    FastLED.addLeds<WS2812B, EFLED_PIN_LED_DATA, GRB>(this->led_data, EFLED_TOTAL_NUM);
    FastLED.setBrightness(this->max_brightness);

    this->enablePower();
}

void EFLed::enablePower() {
    pinMode(EFLED_PIN_5VBOOST_ENABLE, OUTPUT);
    digitalWrite(EFLED_PIN_5VBOOST_ENABLE, HIGH);
    delay(10);
}

void EFLed::disablePower() {
    digitalWrite(EFLED_PIN_5VBOOST_ENABLE, LOW);
    delay(10);
}

void EFLed::clear() {
    for (uint8_t i = 0; i < EFLED_TOTAL_NUM; i++) {
        this->led_data[i] = CRGB::Black;
    }
    FastLED.show();
}

void EFLed::setBrightness(uint8_t brightness) {
    FastLED.setBrightness(round((min(brightness, (uint8_t) 100) / (float) 100) * this->max_brightness));
}

uint8_t EFLed::getBrightness() {
    return (uint8_t) round(FastLED.getBrightness() / (float) this->max_brightness * 100);
}

void EFLed::setDragonNose(const CRGB color) {
    this->led_data[EFLED_DRAGON_NOSE_IDX] = color;
    FastLED.show();
}

void EFLed::setDragonMuzzle(const CRGB color) {
    this->led_data[EFLED_DRAGON_MUZZLE_IDX] = color;
    FastLED.show();
}

void EFLed::setDragonEye(const CRGB color) {
    this->led_data[EFLED_DRAGON_EYE_IDX] = color;
    FastLED.show();
}

void EFLed::setDragonCheek(const CRGB color) {
    this->led_data[EFLED_DRAGON_CHEEK_IDX] = color;
    FastLED.show();
}

void EFLed::setDragonEarBottom(const CRGB color) {
    this->led_data[EFLED_DRAGON_EAR_BOTTOM_IDX] = color;
    FastLED.show();
}

void EFLed::setDragonEarTop(const CRGB color) {
    this->led_data[EFLED_DRAGON_EAR_TOP_IDX] = color;
    FastLED.show();
}

void EFLed::setDragon(const CRGB color[EFLED_DRAGON_NUM]) {
    for (uint8_t i = 0; i < EFLED_DRAGON_NUM; i++) {
        this->led_data[EFLED_DARGON_OFFSET + i] = color[i];
    }
    FastLED.show();
}

void EFLed::setEFBar(const CRGB color[EFLED_EFBAR_NUM]) {
    for (uint8_t i = 0; i < EFLED_EFBAR_NUM; i++) {
        this->led_data[EFLED_EFBAR_OFFSET + i] = color[i];
    }
    FastLED.show();
}

void EFLed::setEFBar(uint8_t idx, const CRGB color) {
    if (idx >= EFLED_EFBAR_NUM) {
        return;
    }

    this->led_data[EFLED_EFBAR_OFFSET + idx] = color;
    FastLED.show();
}
