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
 * @author 32
 */

#include <EFLed.h>
#include <EFLogging.h>

#include "FSMState.h"

#define AUDIO_PIN 14

const int hue_list[] = {
110,110,110,110,110,110,110,110,110,110,110
};

const char* VUMeter::getName() {
    return "VUMeter";
}

bool VUMeter::shouldBeRemembered() {
    return true;
}

void VUMeter::entry() {
    this->tick = 0;
}

int sMin = 0;
int sMax = 4096;

void VUMeter::run() {

    int sample = analogRead(AUDIO_PIN);

    // Update max and min values
    if (sample < sMin) {
        sMin = sample;
    }
    if (sample > sMax) {
        sMax = sample;
    }


    // Calculate peak-to-peak range (this is the signal strength)
    int peakToPeak = sMax - sMin;

    // Map the peak-to-peak value to a range of 0 to NUM_LEDS
    uint8_t n = map(peakToPeak, 0, 4096, 0, 12);


    std::vector<CRGB> dragon = {
        CRGB::Black,
        CHSV(0, 255, 40),
        CHSV(0, 255, 110),
        CHSV(0, 255, 255),
        CRGB::Black,
        CRGB::Black
    };

    std::vector<CRGB> bar = {
        CHSV(hue_list[0], 255, (n > 0 ? 0 : 255)),
        CHSV(hue_list[1], 255, (n > 1 ? 0 : 255)),
        CHSV(hue_list[2], 255, (n > 2 ? 0 : 255)),
        CHSV(hue_list[3], 255, (n > 3 ? 0 : 255)),
        CHSV(hue_list[4], 255, (n > 4 ? 0 : 255)),
        CHSV(hue_list[5], 255, (n > 5 ? 0 : 255)),
        CHSV(hue_list[6], 255, (n > 6 ? 0 : 255)),
        CHSV(hue_list[7], 255, (n > 7 ? 0 : 255)),
        CHSV(hue_list[8], 255, (n > 8 ? 0 : 255)),
        CHSV(hue_list[9], 255, (n > 9 ? 0 : 255)),
        CHSV(hue_list[10], 255, (n > 10 ? 0 : 255)),
    };

    // Calculate current pattern based on tick
    std::rotate(dragon.begin(), dragon.begin() + this->tick % EFLED_DRAGON_NUM, dragon.end());
    std::rotate(bar.rbegin(), bar.rbegin() + this->tick % EFLED_EFBAR_NUM, bar.rend());

    dragon.insert(dragon.end(), bar.begin(), bar.end());
    EFLed.setAll(dragon.data());

    // Prepare next tick
    this->tick++;
}

std::unique_ptr<FSMState> VUMeter::touchEventFingerprintShortpress() {
    if (this->isLocked()) {
        return nullptr;
    }

    return std::make_unique<MenuMain>();
}

std::unique_ptr<FSMState> VUMeter::touchEventFingerprintLongpress() {
    return this->touchEventFingerprintShortpress();
}

std::unique_ptr<FSMState> VUMeter::touchEventFingerprintRelease() {
    if (this->isLocked()) {
        return nullptr;
    }

    this->globals->animMatrixIdx = (this->globals->animMatrixIdx + 1) % 9;
    this->is_globals_dirty = true;
    this->tick = 0;

    return nullptr;
}

std::unique_ptr<FSMState> VUMeter::touchEventAllLongpress() {
    this->toggleLock();
    return nullptr;
}
