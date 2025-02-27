/*
 * Copyright (c) 2013, James Nutaro
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 *
 * Bugs, comments, and questions can be sent to nutaro@gmail.com
 */

#ifndef _adevs_abstract_simulator_h_
#define _adevs_abstract_simulator_h_

#include <memory>
#include "adevs/event_listener.h"
#include "adevs/models.h"

using namespace std;


namespace adevs {

/*
 * This is the base class for all simulators. It defines an interface that is
 * supported by all derived classes and provides some basic helper routines
 * for those derived classes.
 */
template <class OutputType, class TimeType = double>
class AbstractSimulator {
  public:
    /// Constructor
    AbstractSimulator() {}
    /*
     * Add an event listener that will be notified of output events
     * produced by all components within the model.
     * @param l The listener to be notified
     */
    void addEventListener(shared_ptr<EventListener<OutputType, TimeType>> l) {
        listeners.push_back(l);
    }
    /*
     * Remove an event listener that was previously added.
     * @param l The listener to be removed
     */
    void removeEventListener(
        shared_ptr<EventListener<OutputType, TimeType>> l) {
        listeners.erase(l);
    }
    /// Get the model's next event time
    virtual TimeType nextEventTime() = 0;
    /// Execute the simulator until the next event time is greater than tend
    virtual TimeType execUntil(TimeType tend) = 0;
    /// Destructor leaves the model intact.
    virtual ~AbstractSimulator() {}
    /// Notify listeners of an output event.
    void notify_output_listeners(Devs<OutputType, TimeType>* model,
                                 OutputType const &value, TimeType t);
    /// Notify listeners of an input event.
    void notify_input_listeners(Devs<OutputType, TimeType>* model,
                                OutputType const &value, TimeType t);
    /// Notify listeners of a state change.
    void notify_state_listeners(Atomic<OutputType, TimeType>* model,
                                TimeType t);

  private:
    /// Eternal event listeners
    list<shared_ptr<EventListener<OutputType, TimeType>>> listeners;
};

template <class OutputType, class TimeType>
void AbstractSimulator<OutputType, TimeType>::notify_output_listeners(
    Devs<OutputType, TimeType>* model, OutputType const &value, TimeType t) {
    Event<OutputType, TimeType> event(model, value);
    for (auto iter : listeners) {
        iter->outputEvent(event, t);
    }
}

template <class OutputType, class TimeType>
void AbstractSimulator<OutputType, TimeType>::notify_input_listeners(
    Devs<OutputType, TimeType>* model, OutputType const &value, TimeType t) {
    Event<OutputType, TimeType> event(model, value);
    for (auto iter : listeners) {
        iter->inputEvent(event, t);
    }
}

template <class OutputType, class TimeType>
void AbstractSimulator<OutputType, TimeType>::notify_state_listeners(
    Atomic<OutputType, TimeType>* model, TimeType t) {
    for (auto iter : listeners) {
        iter->stateChange(model, t);
    }
}

}  // namespace adevs

#endif
