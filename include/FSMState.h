#ifndef FSM_STATE_H_
#define FSM_STATE_H_

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

#include <memory>

#include "FSMGlobals.h"


/**
 * @brief Base class for FSM states
 */
class FSMState {

    protected:
    
        std::shared_ptr<FSMGlobals> globals;  //!< Pointer to global FSM state variables
        bool is_globals_dirty;                //!< Marks globals as dirty, causing it to be persisted to NVS

    public:
        /**
         * @brief Sets the reference on the global FSM data struct
         */
        void attachGlobals(std::shared_ptr<FSMGlobals> globals);

        /**
         * @brief Determines, if the globals struct was modified and requires
         * to be persisted to NVS by the FSM controller
         */
        bool isGlobalsDirty();

        /**
         * @brief Resets the dirty flag for FSM globals object. Should be called
         * after globals were successfully persisted.
         */
        void resetGlobalsDirty();

        /**
         * @brief Provides access to the name of this state
         * 
         * @return Name of this state
         */
        virtual const char* getName();

        /**
         * @brief If true, the FSM will persist this state and resume to it after
         * reboot, if no other transition to another rememberable state happened since
         * 
         * @return True, if the FSM should remember this state and resume to it upon reboot
         */
        virtual bool shouldBeRemembered();

        /**
         * @brief Provides access to the tick rate of this state
         * 
         * @return Number of milliseconds this state wishes it's run() method to
         * be called periodically. If 0, FSM main tick rate is used.
         */
        virtual const unsigned int getTickRateMs();

        /**
         * @brief Executed on state entry 
         */
        virtual void entry();

        /**
         * @brief Executed once every FSM tick
         */
        virtual void run();

        /**
         * @brief Executd on state exit
         */
        virtual void exit();

        /**
         * @brief Executed on FSMEvent::FingerprintTouch
         */
        virtual std::unique_ptr<FSMState> touchEventFingerprintTouch();

        /**
         * @brief Executed on FSMEvent::FingerprintRelease
         */
        virtual std::unique_ptr<FSMState> touchEventFingerprintRelease();

        /**
         * @brief Executed on FSMEvent::FingerprintShortpress
         */
        virtual std::unique_ptr<FSMState> touchEventFingerprintShortpress();

        /**
         * @brief Executed on FSMEvent::FingerprintLongpress
         */
        virtual std::unique_ptr<FSMState> touchEventFingerprintLongpress();

        /**
         * @brief Executed on FSMEvent::NoseTouch
         */
        virtual std::unique_ptr<FSMState> touchEventNoseTouch();

        /**
         * @brief Executed on FSMEvent::NoseRelease
         */
        virtual std::unique_ptr<FSMState> touchEventNoseRelease();

        /**
         * @brief Executed on FSMEvent::NoseShortpress
         */
        virtual std::unique_ptr<FSMState> touchEventNoseShortpress();

        /**
         * @brief Executed on FSMEvent::NoseLongpress
         */
        virtual std::unique_ptr<FSMState> touchEventNoseLongpress();

};

/**
 * @brief Displays pride flags
 */
struct DisplayPrideFlag : public FSMState {
    uint32_t tick = 0;
    uint8_t flagidx = 0;
    unsigned int switchdelay_ms = 5000;

    virtual const char* getName() override;
    virtual bool shouldBeRemembered() override;
    virtual const unsigned int getTickRateMs() override;

    virtual void entry() override;
    virtual void run() override;

    virtual std::unique_ptr<FSMState> touchEventFingerprintShortpress() override;
    virtual std::unique_ptr<FSMState> touchEventFingerprintRelease() override;
};

/**
 * @brief Displays rainbow animations
 */
struct AnimateRainbow : public FSMState {
    uint32_t tick = 0;

    virtual const char* getName() override;
    virtual bool shouldBeRemembered() override;
    virtual const unsigned int getTickRateMs() override;

    virtual void entry() override;
    virtual void run() override;

    virtual std::unique_ptr<FSMState> touchEventFingerprintShortpress() override;
    virtual std::unique_ptr<FSMState> touchEventFingerprintRelease() override;

    void _animateRainbow();
    void _animateRainbowCircle();
};

/**
 * @brief Displays matrix animation
 */
struct AnimateMatrix : public FSMState {
    uint32_t tick = 0;

    virtual const char* getName() override;
    virtual bool shouldBeRemembered() override;
    virtual const unsigned int getTickRateMs() override;

    virtual void entry() override;
    virtual void run() override;

    virtual std::unique_ptr<FSMState> touchEventFingerprintShortpress() override;

    std::unique_ptr<FSMState> touchEventFingerprintRelease();
};

/**
 * @brief Displays snake animations
 */
struct AnimateSnake : public FSMState {
    uint32_t tick = 0;

    virtual const char* getName() override;
    virtual bool shouldBeRemembered() override;
    virtual const unsigned int getTickRateMs() override;

    virtual void entry() override;
    virtual void run() override;

    virtual std::unique_ptr<FSMState> touchEventFingerprintShortpress() override;
    virtual std::unique_ptr<FSMState> touchEventFingerprintRelease() override;

    void _animateSnake();
    void _animateKnightRider();
};

/**
 * @brief Displays pulsing color
 */
struct AnimateHeartbeat : public FSMState {
    uint32_t tick = 0;

    virtual const char* getName() override;
    virtual bool shouldBeRemembered() override;
    virtual const unsigned int getTickRateMs() override;

    virtual void entry() override;
    virtual void run() override;

    virtual std::unique_ptr<FSMState> touchEventFingerprintShortpress() override;
    virtual std::unique_ptr<FSMState> touchEventNoseRelease() override;
    virtual std::unique_ptr<FSMState> touchEventNoseShortpress() override;
};

/**
 * @brief Accept and handle OTA updates
 */
struct OTAUpdate : public FSMState {
    virtual const char* getName() override;

    virtual void entry() override;
    virtual void run() override;
    virtual void exit() override;

    virtual std::unique_ptr<FSMState> touchEventFingerprintShortpress() override;
};

/**
 * @brief Menu entry point
 */
struct MenuMain : public FSMState {
    uint8_t menucursor_idx = 0;
    uint32_t tick = 0;

    virtual const char* getName() override;
    virtual const unsigned int getTickRateMs() override;

    virtual void entry() override;
    virtual void run() override;
    virtual void exit() override;

    virtual std::unique_ptr<FSMState> touchEventFingerprintRelease() override;
    virtual std::unique_ptr<FSMState> touchEventFingerprintShortpress() override;
    virtual std::unique_ptr<FSMState> touchEventFingerprintLongpress() override;
    virtual std::unique_ptr<FSMState> touchEventNoseLongpress() override;
};

#endif /* FSM_STATE_H_ */