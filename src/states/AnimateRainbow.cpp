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

#define ANIMATE_RAINBOW_NUM_TOTAL 3  //!< Number of available animations

/**
 * @brief Index of all animations, each consisting of a periodically called
 * animation function and an associated tick rate in milliseconds.
 */
const struct {
    void (AnimateRainbow::* animate)();
    const unsigned int tickrate;
} animations[ANIMATE_RAINBOW_NUM_TOTAL] = {
    {.animate = &AnimateRainbow::_animateRainbowCircle, .tickrate = 20},
    {.animate = &AnimateRainbow::_animateRainbow, .tickrate = 100},
    {.animate = &AnimateRainbow::_animateRainbow, .tickrate = 20},

};

const char* AnimateRainbow::getName() {
    return "AnimateRainbow";
}

bool AnimateRainbow::shouldBeRemembered() {
    return true;
}

const unsigned int AnimateRainbow::getTickRateMs() {
    return animations[this->globals->animRainbowIdx % ANIMATE_RAINBOW_NUM_TOTAL].tickrate;
}

void AnimateRainbow::entry() {
    this->tick = 0;
}

void AnimateRainbow::run() {
    (*this.*(animations[this->globals->animRainbowIdx % ANIMATE_RAINBOW_NUM_TOTAL].animate))();
    this->tick++;
}

std::unique_ptr<FSMState> AnimateRainbow::touchEventFingerprintRelease() {
    this->globals->animRainbowIdx++;
    this->is_globals_dirty = true;
    this->tick = 0;
    EFLed.clear();

    LOGF_INFO(
        "(AnimateRainbow) Changed animation mode to: %d\r\n",
        this->globals->animRainbowIdx % ANIMATE_RAINBOW_NUM_TOTAL
    );

    return nullptr;
}

std::unique_ptr<FSMState> AnimateRainbow::touchEventFingerprintShortpress() {
    return std::make_unique<MenuMain>();
}

void AnimateRainbow::_animateRainbow() {
    EFLed.setAllSolid(CHSV((tick % 256), 255, 255));
}

void AnimateRainbow::_animateRainbowCircle() {
    CRGB data[EFLED_TOTAL_NUM];
    fill_rainbow_circular(data, EFLED_TOTAL_NUM, (tick % 128)*2, true);
    EFLed.setAll(data);
}
