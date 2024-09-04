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

#include "FSMState.h"

const char *AnimateHeartbeat::getName() {
    return "AnimateHeartbeat";
}

bool AnimateHeartbeat::shouldBeRemembered() {
    return true;
}

const unsigned int AnimateHeartbeat::getTickRateMs() {
    return 70;
}

void AnimateHeartbeat::entry() {
    currentHue = random(0, 255);
    this->tick = 0;
}

void AnimateHeartbeat::run() {
    CRGB data[EFLED_TOTAL_NUM];
    fill_solid(data, EFLED_TOTAL_NUM, CRGB::Black);

    for (uint8_t i = 0; i < EFLED_TOTAL_NUM; i++) {
        float dx = EFLedClass::getLEDPosition(i).x - EFLedClass::getLEDPosition(EFLED_DRAGON_EYE_IDX).x;
        float dy = EFLedClass::getLEDPosition(i).y - EFLedClass::getLEDPosition(EFLED_DRAGON_EYE_IDX).y;
        float distance = sqrt(dx * dx + dy * dy);

        // LEDs further away from the eye lag behind
        float t = tick / 40.0 - distance / 80.0;

        float intensity = sin(t * 1.0 * M_PI);
        intensity = intensity < 0.0 ? 0.0 : intensity;

        uint8_t value = static_cast<uint8_t>(intensity * 255);
        data[i] = CHSV(currentHue, 255, value);
    }

    EFLed.setAll(data);

    // Prepare next tick
    this->tick = this->tick + currentTickIncrease + 1;
}

std::unique_ptr<FSMState> AnimateHeartbeat::touchEventFingerprintShortpress() {
    return std::make_unique<MenuMain>();
}

std::unique_ptr<FSMState> AnimateHeartbeat::touchEventNoseRelease() {
    uint8_t oldHue = currentHue;
    currentHue = random(0, 255);
    if (abs(oldHue - currentHue) < 20) {
        // If random is too close to last one, make it more different
        currentHue = (currentHue + 20) % 255;
    }
    EFLed.setDragonEye(CRGB::Black);
    delay(100);
    EFLed.setDragonEye(CHSV(currentHue, 255, 255));
    delay(300);
    EFLed.setDragonEye(CRGB::Black);

    this->tick = 0;
    return nullptr;
}

std::unique_ptr<FSMState> AnimateHeartbeat::touchEventNoseShortpress() {
    currentTickIncrease = (currentTickIncrease + 1) % 3;

    CRGB data[EFLED_EFBAR_NUM];
    fill_solid(data, EFLED_EFBAR_NUM, CRGB::Black);
    EFLed.setEFBar(data);
    delay(100);
    fill_solid(data, currentTickIncrease + 1, CRGB::Red);
    EFLed.setEFBar(data);
    delay(300);
    fill_solid(data, currentTickIncrease + 1, CRGB::Black);
    EFLed.setEFBar(data);
    delay(200);
    fill_solid(data, currentTickIncrease + 1, CRGB::Red);
    EFLed.setEFBar(data);
    delay(400);

    return nullptr;
}
