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

#include "EFTouchZone.h"

#define EFTOUCH_PIN_TOUCH_FINGERPRINT 3
#define EFTOUCH_PIN_TOUCH_NOSE 1

#define EFTOUCH_CALIBRATE_NUM_SAMPLES 10
#define EFTOUCH_SHORTPRESS_DURATION_MS 1000
#define EFTOUCH_LONGPRESS_DURATION_MS  2500

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

        unsigned long last_touch_millis_fingerprint;  //!< Timestamp when the fingerprint was last touched
        unsigned long last_touch_millis_nose;         //!< Timestamp when the nose was last touched

        void (*onFingerprintTouchIsr)(void);       //!< ISR to execute if the fingerprint is first touched
        void (*onFingerprintReleaseIsr)(void);     //!< ISR to execute if the fingerprint is fully released
        void (*onFingerprintShortpressIsr)(void);  //!< ISR to execute if the fingerprint was held for at least a short amount of time
        void (*onFingerprintLongpressIsr)(void);   //!< ISR to execute if the fingerprint was held for a long amount of time

        void (*onNoseTouchIsr)(void);              //!< ISR to execute if the nose is first touched
        void (*onNoseReleaseIsr)(void);            //!< ISR to execute if the nose is fully released
        void (*onNoseShortpressIsr)(void);         //!< ISR to execute if the nose was held for at least a short amount of time
        void (*onNoseLongpressIsr)(void);          //!< ISR to execute if the nose was held for a long amount of time

    public:

        /**
         * @brief Creates a new EFTouch instance with default values
         */
        EFTouchClass();

        /**
         * @brief Destructs this EFTouch instance
         */
        ~EFTouchClass();

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
         * @brief Enable interrupt handling for the given touch zone
         * 
         * @param zone Touch zone to enable interrupts for
         */
        void enableInterrupts(EFTouchZone zone);

        /**
         * @brief Disabled interrupt handling for the given touch zone
         * 
         * @param zone Touch zone to disable interrupts for
         */
        void disableInterrupts(EFTouchZone zone);

        /**
         * @brief Attaches an interrupt service routine (ISR) to the given touch zone
         * that will be executed once the touch zone is first touched
         * 
         * @param zone Touch zone to attach the given ISR to
         * @param isr Interrupt service routine to execute
         */
        void attachInterruptOnTouch(EFTouchZone zone, void ARDUINO_ISR_ATTR (*isr)(void));

        /**
         * @brief Attaches an interrupt service routine (ISR) to the given touch zone
         * that will be executed once the touch zone is fully released
         * 
         * @param zone Touch zone to attach the given ISR to
         * @param isr Interrupt service routine to execute
         */
        void attachInterruptOnRelease(EFTouchZone zone, void ARDUINO_ISR_ATTR (*isr)(void));

        /**
         * @brief Attaches an interrupt service routine (ISR) to the given touch zone
         * that will be executed once the touch zone is fully released and the touch
         * duration was at least EFTOUCH_SHORTPRESS_DURATION_MS long
         * 
         * @param zone Touch zone to attach the given ISR to
         * @param isr Interrupt service routine to execute
         */
        void attachInterruptOnShortpress(EFTouchZone zone, void ARDUINO_ISR_ATTR (*isr)(void));

        /**
         * @brief Attaches an interrupt service routine (ISR) to the given touch zone
         * that will be executed once the touch zone is fully released and the touch
         * duration was at least EFTOUCH_LONGPRESS_DURATION_MS long
         * 
         * @param zone Touch zone to attach the given ISR to
         * @param isr Interrupt service routine to execute
         */
        void attachInterruptOnLongpress(EFTouchZone zone, void ARDUINO_ISR_ATTR (*isr)(void));

        /**
         * @brief Detatches the ISR attached by attachInterruptOnTouch() for the given
         * touch zone, if any.
         * 
         * @param zone Touch zone to detatch the ISR from.
         */
        void detatchInterruptOnTouch(EFTouchZone zone);

        /**
         * @brief Detatches the ISR attached by attachInterruptOnRelease() for the given
         * touch zone, if any.
         * 
         * @param zone Touch zone to detatch the ISR from.
         */
        void detatchInterruptOnRelease(EFTouchZone zone);

        /**
         * @brief Detatches the ISR attached by attachInterruptOnShortpress() for the
         * given touch zone, if any.
         * 
         * @param zone Touch zone to detatch the ISR from.
         */
        void detatchInterruptOnShortpress(EFTouchZone zone);

        /**
         * @brief Detatches the ISR attached by attachInterruptOnLongpress() for the
         * given touch zone, if any.
         * 
         * @param zone Touch zone to detatch the ISR from.
         */
        void detatchInterruptOnLongpress(EFTouchZone zone);

        /**
         * @brief INTERNAL interrupt handler. DO NOT EXECUTE DIRECTLY!
         * 
         * @param zone Touch zone the interrupt was fired for
         * @param raising_flank True, if the touch zone was and remains touched
         */
        void ARDUINO_ISR_ATTR _handleInterrupt(EFTouchZone zone, bool raising_flank);
    
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_EFTOUCH)
extern EFTouchClass EFTouch;
#endif

#endif /* EFTOUCH_H_ */
