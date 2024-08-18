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

const char* DisplayPrideFlag::getName() {
    return "DisplayPrideFlag";
}

const unsigned int DisplayPrideFlag::getTickRateMs() {
    return 3000;
}

void DisplayPrideFlag::run() {
    switch(this->globals->prideFlagModeIdx) {
        case 0:
            // Cycle through all flags
            LOGF_DEBUG("(DisplayPrideFlag) Switched pride flag to: %d\r\n", flagidx);
            switch (flagidx) {
                case 0: EFLed.setEFBar(EFPrideFlags::LGBT); break;
                case 1: EFLed.setEFBar(EFPrideFlags::LGBTQI); break;
                case 2: EFLed.setEFBar(EFPrideFlags::Bisexual); break;
                case 3: EFLed.setEFBar(EFPrideFlags::Polyamorous); break;
                case 4: EFLed.setEFBar(EFPrideFlags::Polysexual); break;
                case 5: EFLed.setEFBar(EFPrideFlags::Transgender); break;
                case 6: EFLed.setEFBar(EFPrideFlags::Pansexual); break;
                case 7: EFLed.setEFBar(EFPrideFlags::Asexual); break;
                case 8: EFLed.setEFBar(EFPrideFlags::Genderfluid); break;
                case 9: EFLed.setEFBar(EFPrideFlags::Genderqueer); break;
                case 10: EFLed.setEFBar(EFPrideFlags::Nonbinary); break;
                case 11: EFLed.setEFBar(EFPrideFlags::Intersex); break;
            }
            flagidx = (flagidx + 1) % 12;
            break;
        // Static flags
        case 1: EFLed.setEFBar(EFPrideFlags::LGBT); break;
        case 2: EFLed.setEFBar(EFPrideFlags::LGBTQI); break;
        case 3: EFLed.setEFBar(EFPrideFlags::Bisexual); break;
        case 4: EFLed.setEFBar(EFPrideFlags::Polyamorous); break;
        case 5: EFLed.setEFBar(EFPrideFlags::Polysexual); break;
        case 6: EFLed.setEFBar(EFPrideFlags::Transgender); break;
        case 7: EFLed.setEFBar(EFPrideFlags::Pansexual); break;
        case 8: EFLed.setEFBar(EFPrideFlags::Asexual); break;
        case 9: EFLed.setEFBar(EFPrideFlags::Genderfluid); break;
        case 10: EFLed.setEFBar(EFPrideFlags::Genderqueer); break;
        case 11: EFLed.setEFBar(EFPrideFlags::Nonbinary); break;
        case 12: EFLed.setEFBar(EFPrideFlags::Intersex); break;
        default:
            LOG_ERROR("(DisplayPrideFlag) Invalid prideFlagModeIdx!")
            break;
    }
}

std::unique_ptr<FSMState> DisplayPrideFlag::touchEventFingerprintShortpress() {
    return std::make_unique<MenuMain>();
}
