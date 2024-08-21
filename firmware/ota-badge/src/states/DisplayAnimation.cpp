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
#include <EFPrideFlags.h>

#include "FSMState.h"

#define DISPLAY_ANIMATION_NUM_TOTAL 3

/**
 * @brief Tick rates of all animations in ms
 */
const unsigned int animation_tick_rates[DISPLAY_ANIMATION_NUM_TOTAL] = {
    250,
    80,
    80
};

const char* DisplayAnimation::getName() {
    return "DisplayAnimation";
}

const unsigned int DisplayAnimation::getTickRateMs() {
    return animation_tick_rates[this->globals->animationModeIdx % DISPLAY_ANIMATION_NUM_TOTAL];
}

void DisplayAnimation::entry() {
    this->tick = 0;
}

void DisplayAnimation::run() {
    switch (this->globals->animationModeIdx % DISPLAY_ANIMATION_NUM_TOTAL) {
        case 0: this->_animateRainbow(); break;
        case 1: this->_animateKnightRider(); break;
        case 2: this->_animateSnake(); break;
    }

    this->tick++;
}

std::unique_ptr<FSMState> DisplayAnimation::touchEventFingerprintRelease() {
    this->globals->animationModeIdx++;
    this->tick = 0;
    EFLed.clear();

    LOGF_INFO(
        "(DisplayAnimation) Changed animation mode to: %d\r\n",
        this->globals->animationModeIdx % DISPLAY_ANIMATION_NUM_TOTAL
    );

    return nullptr;
}

std::unique_ptr<FSMState> DisplayAnimation::touchEventFingerprintShortpress() {
    return std::make_unique<MenuMain>();
}

void DisplayAnimation::_animateRainbow() {
    EFLed.setAllSolid(CHSV((tick % 64) * 4, 255, 255));
}

void DisplayAnimation::_animateKnightRider() {
    // Calculate pattern
    uint16_t pattern = 0b111 << (EFLED_EFBAR_NUM - 3);
    if (tick % 16 < 8) {
        // Animate down
        pattern >>= tick % 8;
    } else {
        // Animate up
        pattern >>= 8 - (tick % 8);
    }

    // Translate pattern to LED colors
    CRGB colors[EFLED_EFBAR_NUM] = {};
    for (uint8_t i = 0; i < EFLED_EFBAR_NUM; i++) {
        if (pattern & 0b1) {
            colors[i] = CRGB::Red;
        }
        pattern >>= 1;
    }

    // Apply
    EFLed.setEFBar(colors);
}

void DisplayAnimation::_animateSnake() {
    std::vector<CRGB> pattern = {
        CRGB::Red,
        CRGB::Red,
        CRGB::Red,
        CRGB::Black,
        CRGB::Black,
        CRGB::Black,
        CRGB::Black,
        CRGB::Black,
        CRGB::Black,
        CRGB::Black,
        CRGB::Black,
        CRGB::Black,
        CRGB::Black,
        CRGB::Black,
        CRGB::Black,
        CRGB::Black,
        CRGB::Black,
    };

    std::rotate(pattern.rbegin(), pattern.rbegin()+this->tick % EFLED_TOTAL_NUM, pattern.rend());
    EFLed.setAll(pattern.data());
}
