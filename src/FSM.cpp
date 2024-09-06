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

#include <EFLed.h>
#include <EFLogging.h>

#include "FSM.h"

Preferences pref;

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
    // Restore FSM data
    this->restoreGlobals();

    // Restore LED brightness setting
    EFLed.setBrightnessPercent(this->globals->ledBrightnessPercent);
    
    // Resume last remembered state
    switch (this->globals->resumeStateIdx) {
        case 0: this->transition(std::make_unique<DisplayPrideFlag>()); break;
        case 1: this->transition(std::make_unique<AnimateRainbow>()); break;
        case 2: this->transition(std::make_unique<AnimateMatrix>()); break;
        case 3: this->transition(std::make_unique<AnimateSnake>()); break;
        case 4: this->transition(std::make_unique<AnimateHeartbeat>()); break;
        default:
            LOGF_WARNING("(FSM) Failed to resume to unknown state: %d\r\n", this->globals->resumeStateIdx);
            this->transition(std::make_unique<DisplayPrideFlag>());
            break;

    }
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
        this->globals->resumeStateIdx = this->globals->menuMainPointerIdx;
    }
    if (this->state->isGlobalsDirty() || next->shouldBeRemembered()) {
        this->persistGlobals();
        this->state->resetGlobalsDirty();
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
    // Handle dirtied FSM globals
    if (this->state->isGlobalsDirty()) {
        this->persistGlobals();
        this->state->resetGlobalsDirty();
    }

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
            case FSMEvent::AllShortpress:
                LOGF_DEBUG("(FSM) Processing Event: AllShortpress@%s\r\n", this->state->getName());
                next = this->state->touchEventAllShortpress();
                break;
            case FSMEvent::AllLongpress:
                LOGF_DEBUG("(FSM) Processing Event: AllLongpress@%s\r\n", this->state->getName());
                next = this->state->touchEventAllLongpress();
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
    pref.begin(this->NVS_NAMESPACE, false);
    LOGF_INFO("(FSM) Persisting FSM state data to NVS area: %s\r\n", this->NVS_NAMESPACE);
    pref.clear();
    LOG_DEBUG("(FSM)  -> Clear storage area");
    pref.putUInt("resumeStateIdx", this->globals->resumeStateIdx);
    LOGF_DEBUG("(FSM)  -> resumeStateIdx = %d\r\n", this->globals->resumeStateIdx);
    pref.putUInt("menuIdx", this->globals->menuMainPointerIdx);
    LOGF_DEBUG("(FSM)  -> menuIdx = %d\r\n", this->globals->menuMainPointerIdx);
    pref.putUInt("prideFlagMode", this->globals->prideFlagModeIdx);
    LOGF_DEBUG("(FSM)  -> prideFlagMode = %d\r\n", this->globals->prideFlagModeIdx);
    pref.putUInt("animRainbow", this->globals->animRainbowIdx);
    LOGF_DEBUG("(FSM)  -> animRainbow = %d\r\n", this->globals->animRainbowIdx);
    pref.putUInt("animSnake", this->globals->animSnakeIdx);
    LOGF_DEBUG("(FSM)  -> animSnake = %d\r\n", this->globals->animSnakeIdx);
    pref.putUInt("animHbHue", this->globals->animHeartbeatHue);
    LOGF_DEBUG("(FSM)  -> animHbHue = %d\r\n", this->globals->animHeartbeatHue);
    pref.putUInt("animHbSpeed", this->globals->animHeartbeatSpeed);
    LOGF_DEBUG("(FSM)  -> animHbSpeed = %d\r\n", this->globals->animHeartbeatSpeed);
    pref.putUInt("animMatrixIdx", this->globals->animMatrixIdx);
    LOGF_DEBUG("(FSM)  -> animMatrixIdx = %d\r\n", this->globals->animMatrixIdx);
    pref.putUInt("ledBrightPcent", this->globals->ledBrightnessPercent);
    LOGF_DEBUG("(FSM)  -> ledBrightPcent = %d\r\n", this->globals->ledBrightnessPercent);
    pref.end();
}

void FSM::restoreGlobals() {
    pref.begin(this->NVS_NAMESPACE, true);
    LOGF_INFO("(FSM) Restoring FSM state data from NVS area: %s\r\n", this->NVS_NAMESPACE);
    this->globals->resumeStateIdx = pref.getUInt("resumeStateIdx", 0);
    LOGF_DEBUG("(FSM)  -> resumeStateIdx = %d\r\n", this->globals->resumeStateIdx);
    this->globals->menuMainPointerIdx = pref.getUInt("menuIdx", 0);
    LOGF_DEBUG("(FSM)  -> menuIdx = %d\r\n", this->globals->menuMainPointerIdx);
    this->globals->prideFlagModeIdx = pref.getUInt("prideFlagMode", 1);
    LOGF_DEBUG("(FSM)  -> prideFlagMode = %d\r\n", this->globals->prideFlagModeIdx);
    this->globals->animRainbowIdx = pref.getUInt("animRainbow", 0);
    LOGF_DEBUG("(FSM)  -> animRainbow = %d\r\n", this->globals->animRainbowIdx);
    this->globals->animSnakeIdx = pref.getUInt("animSnake", 0);
    LOGF_DEBUG("(FSM)  -> animSnake = %d\r\n", this->globals->animSnakeIdx);
    this->globals->animHeartbeatHue = pref.getUInt("animHbHue", 0);
    LOGF_DEBUG("(FSM)  -> animHbHue = %d\r\n", this->globals->animHeartbeatHue);
    this->globals->animHeartbeatSpeed= pref.getUInt("animHbSpeed", 1);
    LOGF_DEBUG("(FSM)  -> animHbSpeed = %d\r\n", this->globals->animHeartbeatSpeed);
    this->globals->animMatrixIdx= pref.getUInt("animMatrixIdx", 0);
    LOGF_DEBUG("(FSM)  -> animMatrixIdx = %d\r\n", this->globals->animMatrixIdx);
    this->globals->ledBrightnessPercent= pref.getUInt("ledBrightPcent", 40);
    LOGF_DEBUG("(FSM)  -> ledBrightPcent = %d\r\n", this->globals->ledBrightnessPercent);
    pref.end();
}
