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

#ifndef _adevs_simulator_h_
#define _adevs_simulator_h_

#include <memory>
#include <set>
#include "adevs/abstract_simulator.h"


namespace adevs {

// template <typename ObjectType, typename TimeType>
// Simulator<ObjectType, TimeType>::Simulator(std::set<Devs<ObjectType, TimeType>> models) : AbstractSimulator<ObjectType, TimeType>(),

template <typename ObjectType, typename TimeType = double>
using ModelSet = std::set<std::shared_ptr<Devs<ObjectType, TimeType>>>;

template <typename ObjectType, typename TimeType = double>
class Simulator : public AbstractSimulatr<ObjectType, TimeType> {

  public:
    Simulator<ObjectType, TimeType>(ModelSet &models);
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
    ModelSet connected_models;
    TimeType simulation_time;
};

template <typename ObjectType, typename TimeType>
Simulator<ObjectType, TimeType>::Simulator(ModelSet &active)
    : AbstractSimulator<ObjectType, TimeType>() {
    // // The Atomic constructor sets the atomic model's
    // // tL correctly to zero, and so it is sufficient
    // // to only worry about putting models with a
    // // non infinite time advance into the schedule.
    // for (typename std::list<Devs<OutputType, TimeType>*>::iterator iter = active.begin();
    //      iter != active.end();
    //      iter++) {
    //   schedule(*iter, adevs_zero<T>());
    // }
}

template <typename ObjectType, typename TimeType = double>
Simulator<ObjectType, TimeType>::Simulator()
    : AbstractSimulator<ObjectType, TimeType> {}

void addEventListener(EventListener<OutputType, TimeType>* l) {
    listeners.push_back(l);
}

/*
     * Remove an event listener that was previously added.
     * @param l The listener to be removed
     */
void removeEventListener(EventListener<OutputType, TimeType>* l) {
    listeners.erase(l);
}

}  // namespace adevs

#endif
