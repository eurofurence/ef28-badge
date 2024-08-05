#ifndef EFTOUCH_H_
#define EFTOUCH_H_

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

#define EFTOUCH_PIN_TOUCH_FINGERPRINT 3
#define EFTOUCH_PIN_TOUCH_NOSE 1

#define EFTOUCH_CALIBRATE_NUM_SAMPLES 10


/**
 * @brief Driver for touch sensors
 */
class EFTouchClass {

    protected:

        uint8_t pin_fingerprint;  //!< Pin the fingerprint touch pad is connected to
        uint8_t pin_nose;         //!< Pin the nose touch pad is connected to

        touch_value_t detection_step;  //!< Value change required per registered touch intensity level

        touch_value_t noise_fingerprint;  //!< Calibrated noise floor for fingerprint touch pad
        touch_value_t noise_nose;         //!< Calibrated noise floor for nose touch pad 


    public:

        /**
         * @brief Creates a new EFTouch instance with default values
         */
        EFTouchClass();


        /**
         * @brief Inizializes EFTouch. Sensible default values are used for detection_step
         * and pin selection.
         */
        void init();

        /**
         * @brief Initialized EFTouch
         * 
         * @param detection_step Value change required per registered touch intensity level
         * @param pin_fingerprint Pin the fingerprint touch pad is connected to
         * @param pin_nose Pin the nose touch pad is connected to 
         */
        void init(touch_value_t detection_step, uint8_t pin_fingerprint, uint8_t pin_nose);

        /**
         * @brief Calibrates the noise floor for the analog touch pin readings.
         */
        void calibrate();

        /**
         * @brief Retrieves the current calibrated noise floor value for the
         * fingerprint touch pad.
         * 
         * @return Calibrated touch value for noise floor. 0 if uncalibrated.
         */
        touch_value_t getFingerprintNoiseLevel();

        /**
         * @brief Retrieves the current calibrated noise floor value for the
         * nose touch pad.
         * 
         * @return Calibrated touch value for noise floor. 0 if uncalibrated.
         */
        touch_value_t getNoseNoiseLevel();

        /**
         * @brief Determines if the fingerprint is touched
         * 
         * @return True, if fingerprint is touched
         */
        bool isFingerprintTouched();

        /**
         * @brief Determines if the nose is touched
         * 
         * @return True, if nose is touched
         */
        bool isNoseTouched();

        /**
         * @brief Reads the touch intensity of the fingerprint pad
         * 
         * @return =0 for no touch. >= 1 for touch, increasing with touch intensity 
         */
        uint8_t readFingerprint();

        /**
         * @brief Reads the touch intensity of the nose pad
         * 
         * @return =0 for no touch. >= 1 for touch, increasing with touch intensity 
         */
        uint8_t readNose();

        /** 
         * @brief Attaches an interrupt service routine (ISR) to execute when a touch
         * on the fingerprint pad is registered.
         * 
         * @param isr Function to execute on touch event
         */
        void attachInterruptFingerprint(void (*isr)(void));

        /** 
         * @brief Attaches an interrupt service routine (ISR) to execute when a touch
         * on the nose pad is registered.
         * 
         * @param isr Function to execute on touch event
         */
        void attachInterruptNose(void (*isr)(void));
    
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EFTOUCH)
extern EFTouchClass EFTouch;
#endif

#endif /* EFTOUCH_H_ */
