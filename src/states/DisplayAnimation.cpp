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

#define DISPLAY_ANIMATION_NUM_TOTAL 6  //!< Number of available animations

/**
 * @brief Index of all animations, each consisting of a periodically called
 * animation function and an associated tick rate in milliseconds.
 */
const struct {
    void (DisplayAnimation::* animate)();
    const unsigned int tickrate;
} animations[DISPLAY_ANIMATION_NUM_TOTAL] = {
    {.animate = &DisplayAnimation::_animateRainbowCircle, .tickrate = 20},
    {.animate = &DisplayAnimation::_animateRainbow, .tickrate = 100},
    {.animate = &DisplayAnimation::_animateRainbow, .tickrate = 20},
    {.animate = &DisplayAnimation::_animateMatrix, .tickrate = 100},
    {.animate = &DisplayAnimation::_animateKnightRider, .tickrate = 80},
    {.animate = &DisplayAnimation::_animateSnake, .tickrate = 80}

};

const char* DisplayAnimation::getName() {
    return "DisplayAnimation";
}

bool DisplayAnimation::shouldBeRemembered() {
    return true;
}

const unsigned int DisplayAnimation::getTickRateMs() {
    return animations[this->globals->animationModeIdx % DISPLAY_ANIMATION_NUM_TOTAL].tickrate;
}

void DisplayAnimation::entry() {
    this->tick = 0;
}

void DisplayAnimation::run() {
    (*this.*(animations[this->globals->animationModeIdx % DISPLAY_ANIMATION_NUM_TOTAL].animate))();
    this->tick++;
}

std::unique_ptr<FSMState> DisplayAnimation::touchEventFingerprintRelease() {
    this->globals->animationModeIdx++;
    this->is_globals_dirty = true;
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
    EFLed.setAllSolid(CHSV((tick % 256), 255, 255));
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

void DisplayAnimation::_animateMatrix() {
    std::vector<CRGB> dragon = {
        CRGB::Black,
        CHSV(95, 255, 110),
        CHSV(95, 255, 255),
        CHSV(95, 255, 110),
        CRGB::Black,
        CRGB::Black
    };

    std::vector<CRGB> bar = {
        CHSV(95, 255, 110),
        CHSV(95, 255, 255),
        CHSV(95, 255, 110),
        CRGB::Black,
        CRGB::Black,
        CHSV(95, 255, 110),
        CHSV(95, 255, 255),
        CHSV(95, 255, 110),
        CRGB::Black,
        CRGB::Black,
        CRGB::Black,
    };

    std::rotate(dragon.begin(), dragon.begin() + this->tick % EFLED_DRAGON_NUM, dragon.end());
    std::rotate(bar.rbegin(), bar.rbegin() + this->tick % EFLED_EFBAR_NUM, bar.rend());

    dragon.insert(dragon.end(), bar.begin(), bar.end());
    EFLed.setAll(dragon.data());
}

void DisplayAnimation::_animateRainbowCircle() {
    CRGB data[EFLED_TOTAL_NUM];
    fill_rainbow_circular(data, EFLED_TOTAL_NUM, (tick % 128)*2, true);
    EFLed.setAll(data);
}
