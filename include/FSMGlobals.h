#ifndef FSMGLOBALS_H_
#define FSMGLOBALS_H_

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

#include <Arduino.h>

/**
 * @brief Internal data structure used by the FSM to allows carrying data over
 * between states and allows it to be persisted to the non-volatile storage (NVS).
 *
 * @warning If you want your data to be persisted to NVS, you need to add it to
 * FSM::persistGlobals() and FSM::restoreGlobals() respectively.
 */
typedef struct {
    uint8_t resumeStateIdx = 0;     //!< Index of the state that should be resumed upon reboot
    uint8_t menuMainPointerIdx = 0; //!< MenuMain: Index of the menu cursor

    uint8_t prideFlagModeIdx = 1;   //!< DisplayPrideFlag: Mode selector
    uint8_t animRainbowIdx = 0;     //!< AnimateRainbow: Mode selector
    uint8_t animSnakeIdx = 0;       //!< AnimateSnake: Mode selector
    uint8_t animHeartbeatHue = 0;   //!< AnimateHeartbeat: Hue selector
    uint8_t animHeartbeatSpeed = 1; //!< AnimateHeartbeat: Speed selector

    uint8_t ledBrightnessPercent = 40;  //!< The global maximum brightness of the LEDs
} FSMGlobals;

#endif /* FSMGLOBALS_H_ */
