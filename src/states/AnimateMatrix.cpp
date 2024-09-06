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

const int hue_list[] = {
    130,  // Start with matrix, I mean Eurofurence, green <3
    160,
    200,
    240,
    280,
    320,
    0,
    40,
    80,
};

const char* AnimateMatrix::getName() {
    return "AnimateMatrix";
}

bool AnimateMatrix::shouldBeRemembered() {
    return true;
}

const unsigned int AnimateMatrix::getTickRateMs() {
    return 100;
}

void AnimateMatrix::entry() {
    this->tick = 0;
}

void AnimateMatrix::run() {
    // map the 360 degree hue value to a byte
    int mappedHue = map(hue_list[this->globals->animMatrixIdx], 0, 359, 0, 255);

    std::vector<CRGB> dragon = {
        CRGB::Black,
        CHSV(mappedHue, 255, 40),
        CHSV(mappedHue, 255, 110),
        CHSV(mappedHue, 255, 255),
        CRGB::Black,
        CRGB::Black
    };

    std::vector<CRGB> bar = {
        CHSV(mappedHue, 255, 50),
        CHSV(mappedHue, 255, 110),
        CHSV(mappedHue, 255, 255),
        CRGB::Black,
        CRGB::Black,
        CHSV(mappedHue, 255, 70),
        CHSV(mappedHue, 255, 100),
        CHSV(mappedHue, 255, 200),
        CRGB::Black,
        CRGB::Black,
        CRGB::Black,
    };

    // Calculate current pattern based on tick
    std::rotate(dragon.begin(), dragon.begin() + this->tick % EFLED_DRAGON_NUM, dragon.end());
    std::rotate(bar.rbegin(), bar.rbegin() + this->tick % EFLED_EFBAR_NUM, bar.rend());

    dragon.insert(dragon.end(), bar.begin(), bar.end());
    EFLed.setAll(dragon.data());

    // Prepare next tick
    this->tick++;
}

std::unique_ptr<FSMState> AnimateMatrix::touchEventFingerprintShortpress() {
    if (this->isLocked()) {
        return nullptr;
    }

    return std::make_unique<MenuMain>();
}

std::unique_ptr<FSMState> AnimateMatrix::touchEventFingerprintLongpress() {
    return this->touchEventFingerprintShortpress();
}

std::unique_ptr<FSMState> AnimateMatrix::touchEventFingerprintRelease() {
    if (this->isLocked()) {
        return nullptr;
    }

    this->globals->animMatrixIdx = (this->globals->animMatrixIdx + 1) % 9;
    this->is_globals_dirty = true;
    this->tick = 0;

    return nullptr;
}

std::unique_ptr<FSMState> AnimateMatrix::touchEventAllLongpress() {
    this->toggleLock();
    return nullptr;
}
