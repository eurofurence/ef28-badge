#include <EFTouch.h>

EFTouch::EFTouch(touch_value_t detection_step, uint8_t pin_fingerprint, uint8_t pin_nose) {
    this->detection_step = detection_step;
    this->pin_fingerprint = pin_fingerprint;
    this->pin_nose = pin_nose;

    this->calibrate();
}

EFTouch::EFTouch() {
    this->detection_step = 10000;
    this->pin_fingerprint = EFTOUCH_PIN_TOUCH_FINGERPRINT;
    this->pin_nose = EFTOUCH_PIN_TOUCH_NOSE;

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
