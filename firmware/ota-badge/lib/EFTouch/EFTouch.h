#ifndef EFTOUCH_H_
#define EFTOUCH_H_

#include <Arduino.h>

#define EFTOUCH_PIN_TOUCH_FINGERPRINT 3
#define EFTOUCH_PIN_TOUCH_NOSE 1

#define EFTOUCH_CALIBRATE_NUM_SAMPLES 10

class EFTouch {

    protected:

        uint8_t pin_fingerprint;  //!< Pin the fingerprint touch pad is connected to
        uint8_t pin_nose;         //!< Pin the nose touch pad is connected to

        touch_value_t detection_step;  //!< Value change required per registered touch intensity level

        touch_value_t noise_fingerprint = 0;  //!< Calibrated noise floor for fingerprint touch pad
        touch_value_t noise_nose = 0;         //!< Calibrated noise floor for nose touch pad 

    public:

        /**
         * @brief Creates a new EFTouch instance with default values
         */
        EFTouch();

        /**
         * @brief Creates a new EFTouch instance
         * 
         * @param detection_step Value change required per registered touch intensity level
         * @param pin_fingerprint Pin the fingerprint touch pad is connected to
         * @param pin_nose Pin the nose touch pad is connected to 
         */
        EFTouch(touch_value_t detection_step, uint8_t pin_fingerprint, uint8_t pin_nose);

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

#endif /* EFTOUCH_H_ */
