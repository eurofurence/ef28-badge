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

#include <EFLed.h>
#include <EFLogging.h>
#include <map>

#include "FSMState.h"

const char *AnimatePerlin::getName() {
    return "AnimatePerlin";
}

bool AnimatePerlin::shouldBeRemembered() {
    return true;
}

const unsigned int AnimatePerlin::getTickRateMs() {
    return 60;
}

void AnimatePerlin::entry() {
    this->tick = random(500);
}

void AnimatePerlin::run() {
    CRGB data[EFLED_TOTAL_NUM];
    fill_solid(data, EFLED_TOTAL_NUM, CRGB::Black);

    constexpr uint8_t hueScaleX = 2;
    constexpr uint8_t hueScaleY = 2;
    constexpr uint8_t valScaleX = 10;
    constexpr uint8_t valScaleY = 10;

    for (uint8_t i = 0; i < EFLED_TOTAL_NUM; i++) {

        uint8_t hueF = inoise8(EFLedClass::getLEDPosition(i).x * hueScaleX, EFLedClass::getLEDPosition(i).y * hueScaleY, tick * 2 * (this->globals->animPerlinSpeed + 1));
        uint16_t valueF = inoise8(EFLedClass::getLEDPosition(i).x * valScaleX, EFLedClass::getLEDPosition(i).y * valScaleY, tick * 2 * (this->globals->animPerlinSpeed + 1));

        // inoise is +-64 somehow
        //int n = inoise16(x << 8, y << 8) >> 8;
        valueF = valueF + 50;
        if (valueF > 255) {
            valueF = 255;
        }
        data[i] = CHSV((hueF * 2) % 255, 255, static_cast<uint8_t>(valueF));
    }

    EFLed.setAll(data);

    // Prepare next tick
    this->tick = this->tick + 1;
}

std::unique_ptr<FSMState> AnimatePerlin::touchEventFingerprintShortpress() {
    return std::make_unique<MenuMain>();
}

std::unique_ptr<FSMState> AnimatePerlin::touchEventNoseShortpress() {
    this->globals->animPerlinSpeed = (this->globals->animPerlinSpeed + 1) % 3;
    this->is_globals_dirty = true;

    CRGB data[EFLED_EFBAR_NUM];
    fill_solid(data, EFLED_EFBAR_NUM, CRGB::Black);
    EFLed.setEFBar(data);
    delay(100);
    fill_solid(data, this->globals->animPerlinSpeed + 1, CRGB::Red);
    EFLed.setEFBar(data);
    delay(300);
    fill_solid(data, this->globals->animPerlinSpeed + 1, CRGB::Black);
    EFLed.setEFBar(data);
    delay(200);
    fill_solid(data, this->globals->animPerlinSpeed + 1, CRGB::Red);
    EFLed.setEFBar(data);
    delay(400);

    return nullptr;
}
