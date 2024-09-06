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

#include <EFLed.h>
#include <EFLogging.h>

#include "FSMState.h"

/**
 * @brief Number of registered menu items
 */
#define MENUMAIN_NUM_MENU_ITEMS 6

CRGB menuColors[11] = {
    CRGB(40,10,10),
    CRGB(10, 40,10),
    CRGB(10,10, 40),
    CRGB(40, 40,10),
    CRGB(40,10, 40),
    CRGB(10, 40, 40),
    CRGB(20, 20, 40),
    CRGB(40, 20, 20),
    CRGB(20, 40, 20),
    CRGB(40, 40, 20),
    CRGB(20, 40, 40)
};

const char *MenuMain::getName() {
    return "MenuMain";
}

const unsigned int MenuMain::getTickRateMs() {
    return 100;
}

void MenuMain::entry() {
    EFLed.clear();
    EFLed.setDragonCheek(CRGB::Green);
    EFLed.setEFBarCursor(this->globals->menuMainPointerIdx, CRGB::Silver, CRGB::Black);
    this->tick = 0;
}

void MenuMain::run() {
    CRGB cursorColor = tick % 6 < 2 ? CRGB::Silver : CRGB::DarkBlue;
    EFLed.setEFBarCursor(this->globals->menuMainPointerIdx, cursorColor, menuColors[this->globals->menuMainPointerIdx]);

    this->tick++;
}

void MenuMain::exit() {
    EFLed.clear();
}

std::unique_ptr<FSMState> MenuMain::touchEventFingerprintRelease() {
    this->globals->menuMainPointerIdx = (this->globals->menuMainPointerIdx + 1) % MENUMAIN_NUM_MENU_ITEMS;
    EFLed.setEFBarCursor(this->globals->menuMainPointerIdx, CRGB::Purple, menuColors[this->globals->menuMainPointerIdx]);
    return nullptr;
}

std::unique_ptr<FSMState> MenuMain::touchEventFingerprintShortpress() {
    LOGF_DEBUG("(MenuMain) menuMainPointerIdx = %d\r\n", this->globals->menuMainPointerIdx);
    switch (this->globals->menuMainPointerIdx) {
        // NOTE: Increase MENUMAIN_NUM_MENU_ITEMS define at the top of this file
        case 0: return std::make_unique<DisplayPrideFlag>();
        case 1: return std::make_unique<AnimateRainbow>();
        case 2: return std::make_unique<AnimateMatrix>();
        case 3: return std::make_unique<AnimateSnake>();
        case 4: return std::make_unique<AnimateHeartbeat>();
        case 5: return std::make_unique<OTAUpdate>();
        default: return nullptr;
    }
}

std::unique_ptr<FSMState> MenuMain::touchEventFingerprintLongpress() {
    return this->touchEventFingerprintShortpress();
}

std::unique_ptr<FSMState> MenuMain::touchEventNoseLongpress() {
    EFLed.clear();
    EFLed.setDragonEye(CRGB::White);

    uint8_t currentBrightness = this->globals->ledBrightnessPercent;
    // if we start at 10, it will be 10 -> 40 -> 70 -> 100 -> 10…
    uint8_t newBrightness =  currentBrightness + 30;
    if (newBrightness > 100) {
        // wrap over to minimum brightness again
        newBrightness = 10;
    }
    LOGF_DEBUG("(MenuMain) Setting brightness percent to %d\r\n", newBrightness);

    // animate to new brightness
    CRGB data[EFLED_EFBAR_NUM];
    fill_solid(data, EFLED_EFBAR_NUM, CRGB::Black);
    float stepSize = (newBrightness - currentBrightness) / 10.0f;
    fill_solid(data, map(currentBrightness, 0, 100, 0, EFLED_EFBAR_NUM), CRGB(30, 30, 30));
    EFLed.setEFBar(data);
    delay(100);
    fill_solid(data, map(currentBrightness, 0, 100, 0, EFLED_EFBAR_NUM), CRGB(100, 100, 100));
    EFLed.setEFBar(data);
    delay(200);
    for(int8_t i = 1; i <= 10; i++) {
        float interpolatedBrightness = currentBrightness + (i * stepSize);
        EFLed.setBrightnessPercent(interpolatedBrightness);
        fill_solid(data, EFLED_EFBAR_NUM, CRGB::Black);
        fill_solid(data, map(interpolatedBrightness, 0, 100, 0, EFLED_EFBAR_NUM), CRGB(100, 100, 100));
        EFLed.setEFBar(data);
        delay(40);
    }
    fill_solid(data, map(newBrightness, 0, 100, 0, EFLED_EFBAR_NUM), CRGB(100, 100, 100));
    EFLed.setEFBar(data);
    delay(400);

    this->globals->ledBrightnessPercent = newBrightness;
    this->is_globals_dirty = true;
    EFLed.setBrightnessPercent(this->globals->ledBrightnessPercent);

    // reset view
    this->entry();
    return nullptr;
}
