#ifndef FSM_H_
#define FSM_H_

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
#include <queue>

#include "FSMState.h"

/**
 * @brief Events the FSM is sensitive to
 */
enum class FSMEvent {
    NoOp,
    FingerprintTouch,
    FingerprintRelease,
    FingerprintShortpress,
    FingerprintLongpress,
    NoseTouch,
    NoseRelease,
    NoseShortpress,
    NoseLongpress,
};

/**
 * @brief Main finite state machine (FSM)
 */
class FSM {
    
    protected:

        unsigned int tickrate_ms;         //!< Amount of milliseconds this FSM whishes to be handle()'ed
        unsigned int state_last_run;      //!< Timestamp of the last execution of the current states run() method

        std::unique_ptr<FSMState> state;  //!< Current FSM state
        std::queue<FSMEvent> eventqueue;  //!< Queue to store FSMEvents. ATTENTION: THIS IS NOT THREAD SAFE ON ITS OWN!

        /**
         * @brief Retrieves the next FSMEvent from the queue in a non-blocking fashion.
         * 
         * @return Next FSMEvent or FSMEvent::NoOp if no events exist
         */
        FSMEvent dequeueEvent();

    public:

        /**
         * @brief Constructs a new main FSM
         * 
         * @param tickrate_ms Milliseconds this FSM wished to be peroidically handle()'ed after
         */
        FSM(unsigned int tickrate_ms);

        /**
         * @brief Destructs the main FSM. De-initializes all resourecs
         */
        ~FSM();

        /**
         * @brief Retrieves the tick rate of this FSM
         * 
         * @return Tick rate in milliseconds
         */
        unsigned int getTickRateMs();

        /**
         * @brief Enqueues the given event to be handled during the next cycle
         */
        void queueEvent(FSMEvent event);

        /**
         * @brief Retrieves the number of FSMEvents currently waiting to be processed
         * 
         * @return Number of currently queued FSMEvents
         */
        unsigned int getQueueSize();

        /**
         * @brief Execute a processing cycle. Processes all events that are currently queued.
         */
        void handle();

        /**
         * @brief Executes a processing cycle. Only processees num_events at max. If more
         * events are inside the queue, these are handled in the next processing cycle.
         * 
         * @param num_events Maximum number of events from the queue to process during this cycle
         */
        void handle(unsigned int num_events);

};

#endif /* FSM_H_ */