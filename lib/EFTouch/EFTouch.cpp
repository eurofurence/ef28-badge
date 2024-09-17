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

#include <EFLogging.h>

#include "EFTouch.h"

EFTouchClass::EFTouchClass()
: pin_fingerprint(EFTOUCH_PIN_TOUCH_FINGERPRINT)
, pin_nose(EFTOUCH_PIN_TOUCH_NOSE)
, detection_step(10000)
, noise_fingerprint(0)
, noise_nose(0)
, last_release_millis_fingerprint(0)
, last_release_millis_nose(0)
, last_touch_millis_fingerprint(0)
, last_touch_millis_nose(0)
, last_multitouch_long(0)
, last_multitouch_short(0)
, onFingerprintTouchIsr(nullptr)
, onFingerprintReleaseIsr(nullptr)
, onFingerprintShortpressIsr(nullptr)
, onFingerprintLongpressIsr(nullptr)
, onNoseTouchIsr(nullptr)
, onNoseReleaseIsr(nullptr)
, onNoseShortpressIsr(nullptr)
, onNoseLongpressIsr(nullptr)
{
}

EFTouchClass::~EFTouchClass() {
    this->disableInterrupts(EFTouchZone::Fingerprint);
    this->disableInterrupts(EFTouchZone::Nose);
}

void EFTouchClass::init() {
    this->init(10000, EFTOUCH_PIN_TOUCH_FINGERPRINT, EFTOUCH_PIN_TOUCH_NOSE);
}

void EFTouchClass::init(touch_value_t detection_step, uint8_t pin_fingerprint, uint8_t pin_nose) {
    // Reset member values
    this->detection_step = detection_step;
    this->pin_fingerprint = pin_fingerprint;
    this->pin_nose = pin_nose;
    this->last_release_millis_fingerprint = 0;
    this->last_release_millis_nose = 0;
    this->last_touch_millis_fingerprint = 0;
    this->last_touch_millis_nose = 0;
    this->last_multitouch_long = 0;
    this->last_multitouch_short = 0;
    this->onFingerprintTouchIsr = nullptr;
    this->onFingerprintReleaseIsr = nullptr;
    this->onFingerprintShortpressIsr = nullptr;
    this->onFingerprintLongpressIsr = nullptr;
    this->onNoseTouchIsr = nullptr;
    this->onNoseReleaseIsr = nullptr;
    this->onNoseShortpressIsr = nullptr;
    this->onNoseLongpressIsr = nullptr;

    LOGF_INFO("(EFTouch) Initialized EFTouch instance with detection_step=%d\r\n", detection_step);
    LOGF_DEBUG("(EFTouch) Registered: pin_fingerprint=%d pin_nose=%d\r\n", pin_fingerprint, pin_nose);

    this->calibrate();
    this->enableInterrupts(EFTouchZone::Fingerprint);
    this->enableInterrupts(EFTouchZone::Nose);
}

void EFTouchClass::calibrate() {
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
            LOGF_INFO("(EFTouch) Calibrated fingerprint noise floor to: %d\r\n", this->noise_fingerprint);
        }

        // Nose
        reading = touchRead(this->pin_nose);
        if (reading > this->noise_nose) {
            this->noise_nose = reading;
            LOGF_INFO("(EFTouch) Calibrated nose noise floor to: %d\r\n", this->noise_nose);
        }
    }
}

touch_value_t EFTouchClass::getFingerprintNoiseLevel() {
    return this->noise_fingerprint;
}

touch_value_t EFTouchClass::getNoseNoiseLevel() {
    return this->noise_nose;
}

bool EFTouchClass::isFingerprintTouched() {
    return touchRead(this->pin_fingerprint) > this->noise_fingerprint + this->detection_step;
}

bool EFTouchClass::isNoseTouched() {
    return touchRead(this->pin_nose) > this->noise_nose + this->detection_step;
}

uint8_t EFTouchClass::readFingerprint() {
    touch_value_t reading = touchRead(this->pin_fingerprint); 

    if (reading < this->noise_fingerprint + this->detection_step) {
        return 0;
    } else {
        return (reading - this->noise_fingerprint) / this->detection_step;
    }
}

uint8_t EFTouchClass::readNose() {
    touch_value_t reading = touchRead(this->pin_nose); 

    if (reading < this->noise_nose + this->detection_step) {
        return 0;
    } else {
        return (reading - this->noise_nose) / this->detection_step;
    }
}

void ARDUINO_ISR_ATTR _eftouch_isr_fingerprint() {
    EFTouch._handleInterrupt(
        EFTouchZone::Fingerprint,
        touchInterruptGetLastStatus(EFTOUCH_PIN_TOUCH_FINGERPRINT)
    );
}

void ARDUINO_ISR_ATTR _eftouch_isr_nose() {
    EFTouch._handleInterrupt(
        EFTouchZone::Nose,
        touchInterruptGetLastStatus(EFTOUCH_PIN_TOUCH_NOSE)
    );
}

void EFTouchClass::enableInterrupts(EFTouchZone zone) {
    switch (zone) {
        case EFTouchZone::Fingerprint:
            touchAttachInterrupt(
                this->pin_fingerprint,
                _eftouch_isr_fingerprint,
                this->detection_step
            );
            LOG_INFO("(EFTouch) Enabled fingerprint interrupts");
            break;
        case EFTouchZone::Nose:
            touchAttachInterrupt(
                this->pin_nose,
                _eftouch_isr_nose,
                this->detection_step
            );
            LOG_INFO("(EFTouch) Enabled nose interrupts");
            break;
        default:
            LOGF_ERROR("(EFTouch) Cannot enable interrupts for invalid touch zone: %d\r\n", zone);
            break;
    }
}

void EFTouchClass::disableInterrupts(EFTouchZone zone) {
    switch (zone) {
        case EFTouchZone::Fingerprint:
            touchDetachInterrupt(this->pin_fingerprint);
            LOG_INFO("(EFTouch) Disabled fingerprint interrupts");
            break;
        case EFTouchZone::Nose:
            touchDetachInterrupt(this->pin_nose);
            LOG_INFO("(EFTouch) Disabled nose interrupts");
            break;
        default:
            LOGF_ERROR("(EFTouch) Cannot disable interrupts for invalid touch zone: %d\r\n", zone);
            break;
    }
}

void ARDUINO_ISR_ATTR EFTouchClass::_handleInterrupt(EFTouchZone zone, bool raising_flank) {
    switch (zone) {
        case EFTouchZone::Fingerprint:
            if (raising_flank) {
                // Register first touch timestamp
                this->last_touch_millis_fingerprint = millis();

                // Fire onFingerprintTouchIsr()
                if (this->onFingerprintTouchIsr != nullptr) {
                    this->onFingerprintTouchIsr();
                }
            } else {
                // Register last release timestamp
                this->last_release_millis_fingerprint = millis();

                // Fire onFingerprintShortpressIsr() if applicable
                if (this->onFingerprintShortpressIsr != nullptr) {
                    if (this->last_touch_millis_fingerprint + EFTOUCH_SHORTPRESS_DURATION_MS < millis()) {
                        // Check if all touch zones were short pressed 
                        if (this->onAllShortpressIsr != nullptr && (this->last_multitouch_short + EFTOUCH_MULTITOUCH_COOLDOWN_MS < millis())) {
                            if ((this->isNoseTouched() || millis() - this->last_release_millis_nose < EFTOUCH_MULTITOUCH_COOLDOWN_MS) &&
                                (this->last_touch_millis_nose + EFTOUCH_SHORTPRESS_DURATION_MS < millis()))
                            {
                                this->last_multitouch_short = millis();
                                this->onAllShortpressIsr();
                            }
                        }

                        // Execute zone ISR
                        this->onFingerprintShortpressIsr();
                    }
                }
                // Fire onFingerperintLongpressIsr() if applicable
                if (this->onFingerprintLongpressIsr != nullptr) {
                    if (this->last_touch_millis_fingerprint + EFTOUCH_LONGPRESS_DURATION_MS < millis()) {
                        // Check if all touch zones were long pressed 
                        if (this->onAllLongpressIsr != nullptr && (this->last_multitouch_long + EFTOUCH_MULTITOUCH_COOLDOWN_MS < millis())) {
                            if ((this->isNoseTouched() || millis() - this->last_release_millis_nose < EFTOUCH_MULTITOUCH_COOLDOWN_MS ) &&
                                (this->last_touch_millis_nose + EFTOUCH_LONGPRESS_DURATION_MS < millis()))
                            {
                                this->last_multitouch_long = millis();
                                this->onAllLongpressIsr();
                            }
                        }

                        // Execute zone ISR
                        this->onFingerprintLongpressIsr();
                    }
                }
                // Fire onFingerprintReleaseIsr()
                if (this->onFingerprintReleaseIsr != nullptr) {
                    this->onFingerprintReleaseIsr();
                }
            }
            break;
        case EFTouchZone::Nose:
            if (raising_flank) {
                // Register first touch timestamp
                this->last_touch_millis_nose = millis();

                // Fire onNoseTouchIsr()
                if (this->onNoseTouchIsr != nullptr) {
                    this->onNoseTouchIsr();
                }
            } else {
                // Register last release timestamp
                this->last_release_millis_nose = millis();

                // Fire onNoseShortpressIsr() if applicable
                if (this->onNoseShortpressIsr != nullptr) {
                    if (this->last_touch_millis_nose + EFTOUCH_SHORTPRESS_DURATION_MS < millis()) {
                        // Check if all touch zones were short pressed 
                        if (this->onAllShortpressIsr != nullptr && (this->last_multitouch_short + EFTOUCH_MULTITOUCH_COOLDOWN_MS < millis())) {
                            if ((this->isFingerprintTouched() || millis() - this->last_release_millis_fingerprint < EFTOUCH_MULTITOUCH_COOLDOWN_MS) &&
                                (this->last_touch_millis_fingerprint + EFTOUCH_SHORTPRESS_DURATION_MS < millis()))
                            {
                                this->last_multitouch_short = millis();
                                this->onAllShortpressIsr();
                            }
                        }
                        // Execute zone ISR
                        this->onNoseShortpressIsr();
                    }
                }
                // Fire onFingerperintLongpressIsr() if applicable
                if (this->onNoseLongpressIsr != nullptr) {
                    if (this->last_touch_millis_nose + EFTOUCH_LONGPRESS_DURATION_MS < millis()) {
                        // Check if all touch zones were long pressed 
                        if (this->onAllLongpressIsr != nullptr && (this->last_multitouch_long + EFTOUCH_MULTITOUCH_COOLDOWN_MS < millis())) {
                            if ((this->isFingerprintTouched() || millis() - this->last_release_millis_fingerprint < EFTOUCH_MULTITOUCH_COOLDOWN_MS) &&
                                (this->last_touch_millis_fingerprint + EFTOUCH_LONGPRESS_DURATION_MS < millis()))
                            {
                                this->last_multitouch_long = millis();
                                this->onAllLongpressIsr();
                            }
                        }

                        // Execute zone ISR
                        this->onNoseLongpressIsr();
                    }
                }
                // Fire onNoseReleaseIsr()
                if (this->onNoseReleaseIsr != nullptr) {
                    this->onNoseReleaseIsr();
                }
            }
            break;
    }
}

void EFTouchClass::attachInterruptOnTouch(EFTouchZone zone, void ARDUINO_ISR_ATTR (*isr)(void)) {
    switch (zone) {
        case EFTouchZone::All:
            LOG_ERROR("(EFTouch) Attaching onTouch interrupt to all zones is currently not supported");
            break;
        case EFTouchZone::Fingerprint:
            if (isr) {
                this->onFingerprintTouchIsr = isr;
                LOG_INFO("(EFTouch) Attached onTouch interrupt to fingerprint zone");
            } else {
                this->onFingerprintTouchIsr = nullptr;
                LOG_INFO("(EFTouch) Detached onTouch interrupt from fingerprint zone");
            }
            break;
        case EFTouchZone::Nose:
            if (isr) {
                this->onNoseTouchIsr = isr;
                LOG_INFO("(EFTouch) Attached onTouch interrupt to nose zone");
            } else {
                this->onNoseTouchIsr = nullptr;
                LOG_INFO("(EFTouch) Detatched onTouch interrupt from nose zone");
            }
            break;
        default:
            LOGF_ERROR("(EFTouch) Cannot attach onTouch interrupt to invalid touch zone: %d\r\n", zone);
            break;
    }
}

void EFTouchClass::attachInterruptOnRelease(EFTouchZone zone, void ARDUINO_ISR_ATTR (*isr)(void)) {
    switch (zone) {
        case EFTouchZone::All:
            LOG_ERROR("(EFTouch) Attaching onRelease interrupt to all zones is currently not supported");
            break;
        case EFTouchZone::Fingerprint:
            if (isr) {
                this->onFingerprintReleaseIsr = isr;
                LOG_INFO("(EFTouch) Attached onRelease interrupt to fingerprint zone");
            } else {
                this->onFingerprintReleaseIsr = nullptr;
                LOG_INFO("(EFTouch) Detached onRelease interrupt from fingerprint zone");
            }
            break;
        case EFTouchZone::Nose:
            if (isr) {
                this->onNoseReleaseIsr = isr;
                LOG_INFO("(EFTouch) Attached onRelease interrupt to nose zone");
            } else {
                this->onNoseReleaseIsr = nullptr;
                LOG_INFO("(EFTouch) Detatched onRelease interrupt from nose zone");
            }
            break;
        default:
            LOGF_ERROR("(EFTouch) Cannot attach onRelease interrupt to invalid touch zone: %d\r\n", zone);
            break;
    }
}

void EFTouchClass::attachInterruptOnShortpress(EFTouchZone zone, void ARDUINO_ISR_ATTR (*isr)(void)) {
    switch (zone) {
        case EFTouchZone::All:
            if (isr) {
                this->onAllShortpressIsr = isr;
                LOG_INFO("(EFTouch) Attached onShortpress interrupt to the combined 'All' zone");
            } else {
                this->onAllShortpressIsr = nullptr;
                LOG_INFO("(EFTouch) Detached onShortpress interrupt from the combined 'All' zone");
            }
            break;
        case EFTouchZone::Fingerprint:
            if (isr) {
                this->onFingerprintShortpressIsr = isr;
                LOG_INFO("(EFTouch) Attached onShortpress interrupt to fingerprint zone");
            } else {
                this->onFingerprintShortpressIsr = nullptr;
                LOG_INFO("(EFTouch) Detached onShortpress interrupt from fingerprint zone");
            }
            break;
        case EFTouchZone::Nose:
            if (isr) {
                this->onNoseShortpressIsr = isr;
                LOG_INFO("(EFTouch) Attached onShortpress interrupt to nose zone");
            } else {
                this->onNoseShortpressIsr = nullptr;
                LOG_INFO("(EFTouch) Detatched onShortpress interrupt from nose zone");
            }
            break;
        default:
            LOGF_ERROR("(EFTouch) Cannot attach onShortpress interrupt to invalid touch zone: %d\r\n", zone);
            break;
    }
}

void EFTouchClass::attachInterruptOnLongpress(EFTouchZone zone, void ARDUINO_ISR_ATTR (*isr)(void)) {
    switch (zone) {
        case EFTouchZone::All:
            if (isr) {
                this->onAllLongpressIsr = isr;
                LOG_INFO("(EFTouch) Attached onLongpress interrupt to the combined 'All' touch zone");
            } else {
                this->onAllLongpressIsr = nullptr;
                LOG_INFO("(EFTouch) Detached onLongpress interrupt from the combined 'All' touch zone");
            }
            break;
        case EFTouchZone::Fingerprint:
            if (isr) {
                this->onFingerprintLongpressIsr = isr;
                LOG_INFO("(EFTouch) Attached onLongpress interrupt to fingerprint zone");
            } else {
                this->onFingerprintLongpressIsr = nullptr;
                LOG_INFO("(EFTouch) Detached onLongpress interrupt from fingerprint zone");
            }
            break;
        case EFTouchZone::Nose:
            if (isr) {
                this->onNoseLongpressIsr = isr;
                LOG_INFO("(EFTouch) Attached onLongpress interrupt to nose zone");
            } else {
                this->onNoseLongpressIsr = nullptr;
                LOG_INFO("(EFTouch) Detatched onLongpress interrupt from nose zone");
            }
            break;
        default:
            LOGF_ERROR("(EFTouch) Cannot attach onLongpress interrupt to invalid touch zone: %d\r\n", zone);
            break;
    }
}

void EFTouchClass::detatchInterruptOnTouch(EFTouchZone zone) {
    this->attachInterruptOnTouch(zone, nullptr);
}

void EFTouchClass::detatchInterruptOnRelease(EFTouchZone zone) {
    this->attachInterruptOnRelease(zone, nullptr);
}

void EFTouchClass::detatchInterruptOnShortpress(EFTouchZone zone) {
    this->attachInterruptOnShortpress(zone, nullptr);
}

void EFTouchClass::detatchInterruptOnLongpress(EFTouchZone zone) {
    this->attachInterruptOnLongpress(zone, nullptr);
}


#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EFTOUCH)
EFTouchClass EFTouch;
#endif
