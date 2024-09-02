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

#include "FSMState.h"

void FSMState::attachGlobals(std::shared_ptr<FSMGlobals> globals) {
    this->globals = std::move(globals);
}

bool FSMState::isGlobalsDirty() {
    return this->is_globals_dirty;
}

bool FSMState::shouldBeRemembered() {
    return false;
}

const char* FSMState::getName() {
    return "FSMState";
}

const unsigned int FSMState::getTickRateMs() {
    return 0;
}

void FSMState::entry() {}

void FSMState::run() {}

void FSMState::exit() {}

std::unique_ptr<FSMState> FSMState::touchEventFingerprintTouch() {
    return nullptr;
}

std::unique_ptr<FSMState> FSMState::touchEventFingerprintRelease() {
    return nullptr;
}

std::unique_ptr<FSMState> FSMState::touchEventFingerprintShortpress() {
    return nullptr;
}

std::unique_ptr<FSMState> FSMState::touchEventFingerprintLongpress() {
    return nullptr;
}

std::unique_ptr<FSMState> FSMState::touchEventNoseTouch() {
    return nullptr;
}

std::unique_ptr<FSMState> FSMState::touchEventNoseRelease() {
    return nullptr;
}

std::unique_ptr<FSMState> FSMState::touchEventNoseShortpress() {
    return nullptr;
}

std::unique_ptr<FSMState> FSMState::touchEventNoseLongpress() {
    return nullptr;
}
