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
         * @brief Provides access to the name of this state
         * 
         * @return Name of this state
         */
        virtual const char* getName();

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
 * @brief Main state: Displays a pride flag
 */
struct DisplayPrideFlag : public FSMState {
    uint32_t tick = 0;
    uint8_t flagidx = 0;
    unsigned int switchdelay_ms = 5000;

    virtual const char* getName() override;
    virtual const unsigned int getTickRateMs() override;

    virtual void entry() override;
    virtual void run() override;

    virtual std::unique_ptr<FSMState> touchEventFingerprintShortpress() override;
};

/**
 * @brief Main state: Displays an animation
 */
struct DisplayAnimation : public FSMState {
    uint32_t tick = 0;

    virtual const char* getName() override;
    virtual const unsigned int getTickRateMs() override;

    virtual void entry() override;
    virtual void run() override;

    virtual std::unique_ptr<FSMState> touchEventFingerprintShortpress() override;
    virtual std::unique_ptr<FSMState> touchEventFingerprintRelease() override;

    void _animateKnightRider();
    void _animateMatrix();
    void _animateRainbow();
    void _animateRainbowCircle();
    void _animateSnake();
};

/**
 * @brief Menu entry point
 */
struct MenuMain : public FSMState {
    uint8_t menucursor_idx = 0;

    virtual const char* getName() override;

    virtual void entry() override;
    virtual void exit() override;

    virtual std::unique_ptr<FSMState> touchEventFingerprintTouch() override;
    virtual std::unique_ptr<FSMState> touchEventFingerprintRelease() override;
    virtual std::unique_ptr<FSMState> touchEventFingerprintShortpress() override;
    virtual std::unique_ptr<FSMState> touchEventFingerprintLongpress() override;

    virtual std::unique_ptr<FSMState> touchEventNoseLongpress() override;
};

/**
 * @brief Sub-Menu: Pride flag selector
 */
struct MenuPrideFlagSelector : public FSMState {
    std::unique_ptr<DisplayPrideFlag> prideFlagDisplayRunner;

    virtual const char* getName() override;
    virtual const unsigned int getTickRateMs() override;

    virtual void entry() override;
    virtual void run() override;
    virtual void exit() override;

    virtual std::unique_ptr<FSMState> touchEventFingerprintRelease() override;
    virtual std::unique_ptr<FSMState> touchEventFingerprintShortpress() override;
    virtual std::unique_ptr<FSMState> touchEventFingerprintLongpress() override;
};

/**
 * @brief Sub-Menu: Accept OTA updates
 */
struct MenuOTAUpdate : public FSMState {
    virtual const char* getName() override;

    virtual void entry() override;
    virtual void run() override;
    virtual void exit() override;

    virtual std::unique_ptr<FSMState> touchEventFingerprintShortpress() override;
};

#endif /* FSM_STATE_H_ */