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
#include <Preferences.h>

#include <EFLogging.h>

#include "FSM.h"

FSM::FSM(unsigned int tickrate_ms)
: state(nullptr)
, tickrate_ms(tickrate_ms)
, state_last_run(0)
{
    this->globals = std::make_shared<FSMGlobals>();
    this->state = std::make_unique<DisplayPrideFlag>();
    this->state->attachGlobals(this->globals);
}

FSM::~FSM() {
    this->state->exit();
}

void FSM::resume() {
    this->restoreGlobals();
    
    if (strcmp(this->globals->lastRememberedState, "DisplayPrideFlag") == 0) {
        this->transition(std::make_unique<DisplayPrideFlag>());
        return;
    }

    if (strcmp(this->globals->lastRememberedState, "DisplayAnimation") == 0) {
        this->transition(std::make_unique<DisplayAnimation>());
        return;
    }

    LOGF_WARNING("(FSM) Failed to resume to unknown state: %s\r\n", this->globals->lastRememberedState);
    this->transition(std::make_unique<DisplayPrideFlag>());
}

void FSM::transition(std::unique_ptr<FSMState> next) {
    if (next == nullptr) {
        LOG_WARNING("(FSM) Failed to transition to null state. Aborting.");
        return;
    }

    // State exit
    LOGF_INFO("(FSM) Transition %s -> %s\r\n", this->state->getName(), next->getName());
    this->state->exit();

    // Persist globals if state dirtied it or next state wants to be persisted
    if (next->shouldBeRemembered()) {
        this->globals->lastRememberedState = next->getName();
    }
    if (this->state->isGlobalsDirty() || next->shouldBeRemembered()) {
        this->persistGlobals();
    }

    // Transition to next state
    this->state = std::move(next);
    this->state->attachGlobals(this->globals);
    this->state_last_run = 0;
    this->state->entry();
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
            this->transition(move(next));
        }
    }
}

void FSM::persistGlobals() {
    Preferences pref;
    pref.begin(this->NVS_NAMESPACE, false);
    LOGF_INFO("(FSM) Persisting FSM state data to NVS area: %s\r\n", this->NVS_NAMESPACE);
    pref.clear();
    LOG_DEBUG("(FSM)  -> Clear storage area");
    pref.putString("resumeState", this->globals->lastRememberedState);
    LOGF_DEBUG("(FSM)  -> resumeState = %s\r\n", this->globals->lastRememberedState);
    pref.putUInt("prideFlagMode", this->globals->prideFlagModeIdx);
    LOGF_DEBUG("(FSM)  -> prideFlagMode = %d\r\n", this->globals->prideFlagModeIdx);
    pref.putUInt("animationMode", this->globals->animationModeIdx);
    LOGF_DEBUG("(FSM)  -> animationMode = %d\r\n", this->globals->animationModeIdx);
    pref.end();
}

void FSM::restoreGlobals() {
    Preferences pref;
    pref.begin(this->NVS_NAMESPACE, true);
    LOGF_INFO("(FSM) Restored FSM state data from NVS area: %s\r\n", this->NVS_NAMESPACE);
    this->globals->lastRememberedState = pref.getString("resumeState", "DisplayPrideFlag").c_str();
    LOGF_DEBUG("(FSM)  -> resumeState = %s\r\n", this->globals->lastRememberedState);
    this->globals->prideFlagModeIdx = pref.getUInt("prideFlagMode", 1);
    LOGF_DEBUG("(FSM)  -> prideFlagMode = %d\r\n", this->globals->prideFlagModeIdx);
    this->globals->animationModeIdx = pref.getUInt("animationMode", 0);
    LOGF_DEBUG("(FSM)  -> animationMode = %d\r\n", this->globals->animationModeIdx);
    pref.end();
}
