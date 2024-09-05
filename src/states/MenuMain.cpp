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


const char *MenuMain::getName() {
    return "MenuMain";
}

void MenuMain::entry() {
    EFLed.clear();
    EFLed.setDragonEye(CRGB::Green);
    EFLed.setEFBarCursor(this->globals->menuMainPointerIdx, CRGB::Purple, CRGB::Black);
}

void MenuMain::exit() {
    EFLed.clear();
}

std::unique_ptr<FSMState> MenuMain::touchEventFingerprintRelease() {
    this->globals->menuMainPointerIdx = (this->globals->menuMainPointerIdx + 1) % MENUMAIN_NUM_MENU_ITEMS;
    EFLed.setEFBarCursor(this->globals->menuMainPointerIdx, CRGB::Purple, CRGB::Black);
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
