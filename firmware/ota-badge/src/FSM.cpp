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

#include <EFLogging.h>

#include "FSM.h"

FSM::FSM(unsigned int tickrate_ms)
: state(nullptr)
, tickrate_ms(tickrate_ms)
, state_last_run(0)
{
    this->state = std::make_unique<DisplayPrideFlag>();
    this->state->entry();
}

FSM::~FSM() {
    this->state->exit();
}

unsigned int FSM::getTickRateMs() {
    return this->tickrate_ms;
}

void FSM::queueEvent(FSMEvent event) {
    FSMEvent test = std::move(event);
    noInterrupts();
    {
        this->eventqueue.push(test);
    }
    interrupts();
}

unsigned int FSM::getQueueSize() {
    unsigned int queuesize;

    noInterrupts();
    {
        queuesize = this->eventqueue.size();
    }
    interrupts();

    return queuesize;
}

FSMEvent FSM::dequeueEvent() {
    FSMEvent event = FSMEvent::NoOp;

    noInterrupts();
    {
        if (this->eventqueue.size() > 0) {
            event = std::move(this->eventqueue.front());
            this->eventqueue.pop();
        }
    }
    interrupts();

    return event;
}

void FSM::handle() {
    this->handle(999);
}

void FSM::handle(unsigned int num_events) {
    // Handle state run()
    if (
        this->state->getTickRateMs() == 0 ||
        millis() >= this->state_last_run + this->state->getTickRateMs()
    ) {
        this->state_last_run = millis();
        this->state->run();
    }

    // Handle events
    for (; num_events > 0; num_events--) {
        FSMEvent event = this->dequeueEvent();
        std::unique_ptr<FSMState> next = nullptr;

        // Propagate event to current state
        switch(event) {
            case FSMEvent::FingerprintTouch:
                LOGF_DEBUG("(FSM) Processing Event: FingerprintTouch@%s\r\n", this->state->getName());
                next = this->state->touchEventFingerprintTouch();
                break;
            case FSMEvent::FingerprintRelease:
                LOGF_DEBUG("(FSM) Processing Event: FingerprintRelease@%s\r\n", this->state->getName());
                next = this->state->touchEventFingerprintRelease();
                break;
            case FSMEvent::FingerprintShortpress:
                LOGF_DEBUG("(FSM) Processing Event: FingerprintShortpress@%s\r\n", this->state->getName());
                next = this->state->touchEventFingerprintShortpress();
                break;
            case FSMEvent::FingerprintLongpress:
                LOGF_DEBUG("(FSM) Processing Event: FingerprintLongpress@%s\r\n", this->state->getName());
                next = this->state->touchEventFingerprintLongpress();
                break;
            case FSMEvent::NoseTouch:
                LOGF_DEBUG("(FSM) Processing Event: NoseTouch@%s\r\n", this->state->getName());
                next = this->state->touchEventNoseTouch();
                break;
            case FSMEvent::NoseRelease:
                LOGF_DEBUG("(FSM) Processing Event: NoseRelease@%s\r\n", this->state->getName());
                next = this->state->touchEventNoseRelease();
                break;
            case FSMEvent::NoseShortpress:
                LOGF_DEBUG("(FSM) Processing Event: NoseShortpress@%s\r\n", this->state->getName());
                next = this->state->touchEventNoseShortpress();
                break;
            case FSMEvent::NoseLongpress:
                LOGF_DEBUG("(FSM) Processing Event: NoseLongpress@%s\r\n", this->state->getName());
                next = this->state->touchEventNoseLongpress();
                break;
            case FSMEvent::NoOp:
                return;
            default:
                LOGF_WARNING("(FSM) Failed to handle unknown event: %d\r\n", event);
                return;
        } 

        // Handle state transition
        if (next != nullptr) {
            LOGF_INFO("(FSM) Transition %s -> %s\r\n", this->state->getName(), next->getName());
            this->state->exit();
            this->state = std::move(next);
            this->state_last_run = 0;
            this->state->entry();
        }
    }
}
