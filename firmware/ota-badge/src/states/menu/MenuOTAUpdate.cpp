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

#include <ArduinoOTA.h>

#include <EFBoard.h>
#include <EFLed.h>

#include "constants.h"
#include "secrets.h"

#include "FSMState.h"

const char* MenuOTAUpdate::getName() {
    return "MenuOTAUpdate";
}

void MenuOTAUpdate::entry() {
    // Connecto to wifi
    EFLed.setDragonNose(CRGB::Red);
    if (EFBoard.connectToWifi(WIFI_SSID, WIFI_PASSWORD)) {
        EFLed.setDragonNose(CRGB::Green);
    }

    // Setup OTA
    EFBoard.enableOTA(OTA_SECRET);
    EFLed.setDragonMuzzle(CRGB::Green);
}

void MenuOTAUpdate::run() {
    ArduinoOTA.handle();
}

void MenuOTAUpdate::exit() {
    EFBoard.disableOTA();
    EFBoard.disableWifi();
}

std::unique_ptr<FSMState> MenuOTAUpdate::touchEventFingerprintShortpress() {
    return std::make_unique<MenuMain>();
}