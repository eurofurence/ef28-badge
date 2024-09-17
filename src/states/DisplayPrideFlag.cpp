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

const char* DisplayPrideFlag::getName() {
    return "DisplayPrideFlag";
}

bool DisplayPrideFlag::shouldBeRemembered() {
    return true;
}

const unsigned int DisplayPrideFlag::getTickRateMs() {
    return 20;
}

void DisplayPrideFlag::entry() {
    this->switchdelay_ms = 5000;
    this->tick = 0;
}

void DisplayPrideFlag::run() {
    // Check if we need to switch the flag (Mode: 0)
    if (this->tick % (this->switchdelay_ms / this->getTickRateMs()) == 0) {
        if (this->globals->prideFlagModeIdx == 0) {
            // Cycle through all flags
            LOGF_DEBUG("(DisplayPrideFlag) Switched pride flag to: %d\r\n", flagidx);
            flagidx = (flagidx + 1) % 12;
        }
    }

    // Determine pride flag to show
    const CRGB* prideFlag = EFPrideFlags::LGBTQI;
    if (this->globals->prideFlagModeIdx == 0) {
        switch (flagidx) {
            case 0: prideFlag = EFPrideFlags::LGBTQI; break;
            case 1: prideFlag = EFPrideFlags::LGBT; break;
            case 2: prideFlag = EFPrideFlags::Bisexual; break;
            case 3: prideFlag = EFPrideFlags::Polyamorous; break;
            case 4: prideFlag = EFPrideFlags::Polysexual; break;
            case 5: prideFlag = EFPrideFlags::Transgender; break;
            case 6: prideFlag = EFPrideFlags::Pansexual; break;
            case 7: prideFlag = EFPrideFlags::Asexual; break;
            case 8: prideFlag = EFPrideFlags::Genderfluid; break;
            case 9: prideFlag = EFPrideFlags::Genderqueer; break;
            case 10: prideFlag = EFPrideFlags::Nonbinary; break;
            case 11: prideFlag = EFPrideFlags::Intersex; break;
        }
    } else {
        switch(this->globals->prideFlagModeIdx) {
            // Static flags
            case 1: prideFlag = EFPrideFlags::LGBT; break;
            case 2: prideFlag = EFPrideFlags::LGBTQI; break;
            case 3: prideFlag = EFPrideFlags::Bisexual; break;
            case 4: prideFlag = EFPrideFlags::Polyamorous; break;
            case 5: prideFlag = EFPrideFlags::Polysexual; break;
            case 6: prideFlag = EFPrideFlags::Transgender; break;
            case 7: prideFlag = EFPrideFlags::Pansexual; break;
            case 8: prideFlag = EFPrideFlags::Asexual; break;
            case 9: prideFlag = EFPrideFlags::Genderfluid; break;
            case 10: prideFlag = EFPrideFlags::Genderqueer; break;
            case 11: prideFlag = EFPrideFlags::Nonbinary; break;
            case 12: prideFlag = EFPrideFlags::Intersex; break;
            default:
                LOG_ERROR("(DisplayPrideFlag) Invalid prideFlagModeIdx!")
                break;
        }
    }
    
    // Animate dragon: Rotate current flag to cycle through dragon head
    std::vector<CRGB> rotatedflag(prideFlag, prideFlag + EFLED_EFBAR_NUM);
    std::rotate(rotatedflag.begin(), rotatedflag.begin() + (this->tick % (EFLED_EFBAR_NUM*20)) / 20, rotatedflag.end());

    // Animate dragon: Create current and next dragon head patterns
    CRGB dragon[EFLED_DRAGON_NUM];
    CRGB dragon_next[EFLED_DRAGON_NUM];
    std::copy(rotatedflag.begin(), rotatedflag.begin() + EFLED_DRAGON_NUM, dragon);
    std::copy(rotatedflag.begin() + 1, rotatedflag.begin() + 1 + EFLED_DRAGON_NUM, dragon_next);

    // Animate dragon: Blend both patterns based on current tick and reduce brightness
    CRGB dragon_now[EFLED_DRAGON_NUM];
    blend(dragon, dragon_next, dragon_now, EFLED_DRAGON_NUM, ((this->tick % 20) / 20.0) * 255);
    fadeLightBy(dragon_now, EFLED_DRAGON_NUM, 128);

    // Animate dragon: Finally show it!
    EFLed.setDragon(dragon_now);

    // Refresh flag periodically
    if (this->tick % (this->switchdelay_ms / this->getTickRateMs()) == 0) {
        EFLed.setEFBar(prideFlag);
    }

    // Prepare next tick
    this->tick++;
}

std::unique_ptr<FSMState> DisplayPrideFlag::touchEventFingerprintShortpress() {
    if (this->isLocked()) {
        return nullptr;
    }

    return std::make_unique<MenuMain>();
}

std::unique_ptr<FSMState> DisplayPrideFlag::touchEventFingerprintLongpress() {
    return this->touchEventFingerprintShortpress();
}

std::unique_ptr<FSMState> DisplayPrideFlag::touchEventFingerprintRelease() {
    if (this->isLocked()) {
        return nullptr;
    }

    this->globals->prideFlagModeIdx = (this->globals->prideFlagModeIdx + 1) % 13;
    this->is_globals_dirty = true;
    this->tick = 0;

    return nullptr;
}

std::unique_ptr<FSMState> DisplayPrideFlag::touchEventAllLongpress() {
    this->toggleLock();
    return nullptr;
}
