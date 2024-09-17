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
#include <vector>

#include "FSMState.h"

#define ANIMATE_SNAKE_NUM_TOTAL 4  //!< Number of available animations
#define ANIMATE_HUE_NUM_TOTAL 5   //!< Number of available hues

/**
 * @brief Index of all animations, each consisting of a periodically called
 * animation function and an associated tick rate in milliseconds.
 */
const struct {
    void (AnimateSnake::*animate)();
    const unsigned int tickrate;
} animations[ANIMATE_SNAKE_NUM_TOTAL] = {
    {.animate = &AnimateSnake::_animateSnake, .tickrate = 80},
    {.animate = &AnimateSnake::_animateKnightRider, .tickrate = 80},
    {.animate = &AnimateSnake::_animatePulse, .tickrate = 80},
    {.animate = &AnimateSnake::_animateRandom, .tickrate = 80},
};

const CHSV hueList[] = {
        CHSV(0, 255, 255),
        CHSV(96, 255, 255),
        CHSV(130, 255, 255),
        CHSV(220, 255, 255),
        CHSV(0, 0, 255),
};

int randomLightList[EFLED_TOTAL_NUM] = {};

const char* AnimateSnake::getName() {
    return "AnimateSnake";
}

bool AnimateSnake::shouldBeRemembered() {
    return true;
}

const unsigned int AnimateSnake::getTickRateMs() {
    return animations[this->globals->animSnakeAnimationIdx % ANIMATE_SNAKE_NUM_TOTAL].tickrate;
}

void AnimateSnake::entry() {
    this->tick = 0;
}

void AnimateSnake::run() {
    (*this.*(animations[this->globals->animSnakeAnimationIdx % ANIMATE_SNAKE_NUM_TOTAL].animate))();
    this->tick++;
}

std::unique_ptr<FSMState> AnimateSnake::touchEventFingerprintRelease() {
    if (this->isLocked()) {
        return nullptr;
    }

    this->globals->animSnakeHueIdx++;
    if(this->globals->animSnakeHueIdx == ANIMATE_HUE_NUM_TOTAL) {
        this->globals->animSnakeHueIdx = 0;
        this->globals->animSnakeAnimationIdx++;
        this->globals->animSnakeAnimationIdx %= ANIMATE_SNAKE_NUM_TOTAL;
    }
    this->is_globals_dirty = true;
    this->tick = 0;
    EFLed.clear();

    LOGF_INFO(
        "(AnimateSnake) Changed animation mode to: %d\r\n",
        this->globals->animSnakeAnimationIdx
    );

    return nullptr;
}

std::unique_ptr<FSMState> AnimateSnake::touchEventFingerprintShortpress() {
    if (this->isLocked()) {
        return nullptr;
    }

    return std::make_unique<MenuMain>();
}

std::unique_ptr<FSMState> AnimateSnake::touchEventFingerprintLongpress() {
    return this->touchEventFingerprintShortpress();
}

void AnimateSnake::_animateKnightRider() {
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
            colors[i] = hueList[this->globals->animSnakeHueIdx];
        }
        pattern >>= 1;
    }

    // Apply
    EFLed.setEFBar(colors);
}

void AnimateSnake::_animateSnake() {
    std::vector<CRGB> pattern = {
        hueList[this->globals->animSnakeHueIdx],
        hueList[this->globals->animSnakeHueIdx],
        hueList[this->globals->animSnakeHueIdx],
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

    std::rotate(pattern.rbegin(), pattern.rbegin() + this->tick % EFLED_TOTAL_NUM, pattern.rend());
    EFLed.setAll(pattern.data());
}

std::unique_ptr<FSMState> AnimateSnake::touchEventAllLongpress() {
    this->toggleLock();
    return nullptr;
}

void AnimateSnake::_animatePulse() {

    // Create half of the pattern
    std::vector<CRGB> patternFirstHalf = {
        hueList[this->globals->animSnakeHueIdx],
        CRGB::Black,
        CRGB::Black,
        CRGB::Black,
        CRGB::Black,
    };
    // shift the active LED
    std::rotate(patternFirstHalf.rbegin(), patternFirstHalf.rbegin() + this->tick % 5, patternFirstHalf.rend());

    // copy the pattern and flip it
    std::vector<CRGB> patternSecondHalf = patternFirstHalf;
    std::reverse(patternSecondHalf.begin(), patternSecondHalf.end());

    // Handle the center LED. Only light it in the last part of the animation
    if(patternSecondHalf[0] == hueList[this->globals->animSnakeHueIdx]) {
        patternFirstHalf.insert(patternFirstHalf.end(), hueList[this->globals->animSnakeHueIdx]);
    }else{
        patternFirstHalf.insert(patternFirstHalf.end(), CRGB::Black);
    }

    // create the final pattern array and append the second half pattern
    std::vector<CRGB> pattern = patternFirstHalf;
    pattern.insert(pattern.end(), patternSecondHalf.begin(), patternSecondHalf.end());

    EFLed.setEFBar(pattern.data());
}

void AnimateSnake::_animateRandom() {

    // set a random LED to light up
    if(tick % 2 == 0) {
        randomLightList[random(0, EFLED_TOTAL_NUM-1)] = 255;
    }

    std::vector<CRGB> pattern;

    // loop through all LED brighnesses and set it. Subtract it afterward to slowly dim them
    for(int & i : randomLightList) {
        CHSV color = hueList[this->globals->animSnakeHueIdx];
        color.value = i;
        pattern.insert(pattern.end(), color);
        i -= 20;
        if(i < 0) { i = 0; };
    }


    EFLed.setAll(pattern.data());
}