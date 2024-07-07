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

#include "EFTouch.h"


EFTouch::EFTouch(): EFTouch(10000, EFTOUCH_PIN_TOUCH_FINGERPRINT, EFTOUCH_PIN_TOUCH_NOSE) {}

EFTouch::EFTouch(touch_value_t detection_step, uint8_t pin_fingerprint, uint8_t pin_nose) {
    this->detection_step = detection_step;
    this->pin_fingerprint = pin_fingerprint;
    this->pin_nose = pin_nose;

    this->calibrate();
}

void EFTouch::calibrate() {
    // Reset calibration values
    this->noise_fingerprint = 0;
    this->noise_nose = 0;

    // Calibrate
    touch_value_t reading = 0;
    for (uint8_t i = 0; i < EFTOUCH_CALIBRATE_NUM_SAMPLES; i++) {
        // Fingerprint
        reading = touchRead(this->pin_fingerprint);
        if (reading > this->noise_fingerprint) {
            this->noise_fingerprint = reading;
        }

        // Nose
        reading = touchRead(this->pin_nose);
        if (reading > this->noise_nose) {
            this->noise_nose = reading;
        }
    }
}

touch_value_t EFTouch::getFingerprintNoiseLevel() {
    return this->noise_fingerprint;
}

touch_value_t EFTouch::getNoseNoiseLevel() {
    return this->noise_nose;
}

bool EFTouch::isFingerprintTouched() {
    return touchRead(this->pin_fingerprint) > this->noise_fingerprint + this->detection_step;
}

bool EFTouch::isNoseTouched() {
    return touchRead(this->pin_nose) > this->noise_nose + this->detection_step;
}

uint8_t EFTouch::readFingerprint() {
    touch_value_t reading = touchRead(this->pin_fingerprint); 

    if (reading < this->noise_fingerprint + this->detection_step) {
        return 0;
    } else {
        return (reading - this->noise_fingerprint) / this->detection_step;
    }
}

uint8_t EFTouch::readNose() {
    touch_value_t reading = touchRead(this->pin_nose); 

    if (reading < this->noise_nose + this->detection_step) {
        return 0;
    } else {
        return (reading - this->noise_nose) / this->detection_step;
    }
}

void EFTouch::attachInterruptFingerprint(void (*isr)(void)) {
    touchAttachInterrupt(
        this->pin_fingerprint,
        isr,
        this->detection_step
    );
}

void EFTouch::attachInterruptNose(void (*isr)(void)) {
    touchAttachInterrupt(
        this->pin_nose,
        isr,
         this->detection_step
    );
}
