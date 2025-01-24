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

#include <cassert>
#include <cstdlib>
#include <list>
#include <set>
#include "adevs/abstract_simulator.h"
#include "adevs/event_listener.h"
#include "adevs/models.h"
#include "adevs/sched.h"


namespace adevs {

/*
 * This Simulator class implements the DEVS simulation algorithm.
 * Its methods throw adevs::exception objects if any of the DEVS model
 * constraints are violated (i.e., a negative time advance, a model
 * attempting to send an input directly to itself, or coupled Mealy
 * type systems).
 */
template <class OutputType, class TimeType = double>
class Simulator : public AbstractSimulator<OutputType, TimeType> {

  public:
    /*
     * Create a simulator for a model. The simulator
     * constructor will fail and throw an adevs::exception if the
     * time advance of any component atomic model is less than zero.
     * @param model The model to simulate
     */
    Simulator(shared_ptr<Devs<OutputType, TimeType>> model)
        : AbstractSimulator<OutputType, TimeType>(), io_up_to_date(false) {
        schedule(model.get(), adevs_zero<TimeType>());
    }

    /*
     * Initialize the simulator with a list of models
     * that will be active at the start. The constructor
     * only schedules the models in the list, which avoids
     * the overhead of constructing the full set of
     * models with recursive calls to Network::getComponents().
     * @param active The list of active models.
     */
    Simulator(std::list<shared_ptr<Devs<OutputType, TimeType>>> &active);

    /*
     * Get the model's next event time
     * @return The absolute time of the next event
     */
    TimeType nextEventTime() { return sched.minPriority(); }

    /*
     * Execute the simulation cycle at time nextEventTime()
     * @return The updated simulation time
     */
    TimeType execNextEvent() { return computeNextState(); }

    /*
     * Execute until nextEventTime() > tend.
     * @return The updated simulation time
     */
    TimeType execUntil(TimeType tend) {
        TimeType t = tend + adevs_epsilon<TimeType>();
        while (nextEventTime() <= tend &&
               nextEventTime() < adevs_inf<TimeType>()) {
            t = execNextEvent();
        }
        return t;
    }

    /*
     * Compute the output values of the imminent component models
     * if these values have not already been computed.  This will
     * notify registered EventListeners as the outputs are produced.
     */
    void computeNextOutput();

    /*
     * Compute the output value of the model in response to an input
     * at some time in lastEventTime() <= t <= nextEventTime().
     * This will notify registered EventListeners as the outputs
     * are produced. If this is the first call since the prior
     * state change with the given t, then the new output is computed.
     * Subsequent calls for the same time t simply
     * append to the input already supplied at time t.
     * @param input A list of (input target,value) pairs
     * @param t The time at which the input takes effect
     */
    void computeNextOutput(list<Event<OutputType, TimeType>> &input,
                           TimeType t);

    /*
     * Apply the list of inputs at time t and then compute the next model
     * states. Requires that lastEventTime() <= t <= nextEventTime().
     * This, in effect, implements the state transition function of
     * the resultant model. If the output has already been computed
     * at time t, then the new input at t is simply appended to the
     * prior input. Otherwise, the old results are discarded and input
     * is calculated at the given time.
     * @param input A list of (input target,value) pairs
     * @param t The time at which the input takes effect
     * @return The new, current simulation time
     */

    TimeType computeNextState(list<Event<OutputType, TimeType>> &input,
                              TimeType t);

    /*
     * Compute the next state at the time at the time t and with
     * input supplied at the prior call to computeNextOutput
     * assuming no computeNextState has intervened. Assumes
     * t = nextEventTime() and input an empty list if there was
     * no prior call to computeNextOutput.
     * @return The new, current simulation time
     */
    TimeType computeNextState();

    /*
     * Assign a model to the simulator. This has the same effect as passing
     * the model to the constructor.
     */
    void addModel(shared_ptr<Atomic<OutputType, TimeType>> model) {
        schedule(model.get(), adevs_zero<TimeType>());
    }

    /*
     * Recursively add the model and its elements to the schedule
     * using t as the time of last event.
     */
    void schedule(Devs<OutputType, TimeType>* model, TimeType t);

    /*
     * Recursively remove a model and its components from the schedule
     * and the imminent/activated lists
     */
    void unschedule_model(Devs<OutputType, TimeType>* model);

    set<shared_ptr<Devs<OutputType, TimeType>>> pending_schedule;
    set<shared_ptr<Devs<OutputType, TimeType>>> pending_unschedule;

  private:
    // Bogus input list for execNextEvent() method
    list<Event<OutputType, TimeType>> bogus_input;
    // The event schedule
    Schedule<OutputType, TimeType> sched;
    // List of models that are imminent or activated by input
    list<Atomic<OutputType, TimeType>*> activated;
    // Mealy systems that we need to process
    bool allow_mealy_input;
    bool io_up_to_date;

    TimeType io_time;

    list<MealyAtomic<OutputType, TimeType>*> mealy;


    /// Route an event generated by the source model contained in the parent model.
    void route(Network<OutputType, TimeType>* parent,
               Devs<OutputType, TimeType>* src, OutputType &x);

    /*
     * Add an input to the input list of an an atomic model. If the
     * model is not already active , then this method adds the model to
     * the activated list.
     */
    void inject_event(Atomic<OutputType, TimeType>* model, OutputType &value);

    // Recursively call the model transition of all activated models
    void transition(Devs<OutputType, TimeType>* model,
                    set<Devs<OutputType, TimeType>*> &called);

    /*
     * Delete any thing in the output list, and return the input
     * and output list to the pools.
     * Recursively clean up network model components.
     */
    void clean_up(Devs<OutputType, TimeType>* model);

    /*
     * Construct the complete descendant set of a network model and store it in s.
     */
    void getAllChildren(Network<OutputType, TimeType>* model,
                        set<Devs<OutputType, TimeType>*> &s);
};

template <typename OutputType, typename TimeType>
Simulator<OutputType, TimeType>::Simulator(
    std::list<shared_ptr<Devs<OutputType, TimeType>>> &active)
    : AbstractSimulator<OutputType, TimeType>(), io_up_to_date(false) {
    // The Atomic constructor sets the atomic model's
    // tL correctly to zero, and so it is sufficient
    // to only worry about putting models with a
    // non infinite time advance into the schedule.
    for (auto iter : active) {
        schedule(iter.get(), adevs_zero<TimeType>());
    }
}

template <class OutputType, class TimeType>
void Simulator<OutputType, TimeType>::computeNextOutput(
    list<Event<OutputType, TimeType>> &input, TimeType t) {

    // Undo any prior output calculation at another time
    if (io_up_to_date && !(io_time == t)) {
        typename list<Atomic<OutputType, TimeType>*>::iterator iter;
        for (auto iter : activated) {
            clean_up(iter);
        }
        activated.clear();
    }
    // Input and output happen at the current time.
    io_time = t;
    // Get the imminent Moore models from the schedule if we have not
    // already done so.
    allow_mealy_input = true;
    if (t == sched.minPriority() && !io_up_to_date) {
        // get the list of activated models
        auto activated_models = sched.visitImminent();

        // process moore models
        for (auto model : activated_models) {
            Atomic<OutputType, TimeType>* atmoic_model = model->typeIsAtomic();
            MealyAtomic<OutputType, TimeType>* mealy_model =
                model->typeIsMealyAtomic();

            // need to do this because a MealyAtomic is technically both an
            // Atomic and a MealyAtomic (by inheritance)
            if (atmoic_model != nullptr && mealy_model == nullptr) {
                assert(model->outputs->empty());
                activated.push_back(model);
                model->output_func(*(model->outputs));

                for (auto y_iter : *(model->outputs)) {
                    route(model->getParent(), model, y_iter);
                }
            } else {
                assert(mealy_model != nullptr);
                assert(mealy_model->outputs->empty());
                mealy.push_back(mealy_model);
            }
        }
    }
    // Apply the injected inputs.
    for (auto iter : input) {
        Atomic<OutputType, TimeType>* amodel = iter.model->typeIsAtomic();
        if (amodel != nullptr) {
            inject_event(amodel, iter.value);
        } else {
            route(iter.model->typeIsNetwork(), iter.model, iter.value);
        }
    }
    // Only Moore models can influence Mealy models.
    allow_mealy_input = false;
    // Iterate over activated Mealy models to calculate their output
    for (auto m_iter : mealy) {
        MealyAtomic<OutputType, TimeType>* model = m_iter;
        assert(model->outputs->empty());

        // Put it in the active set if it is not already there
        if (!model->activated) {
            activated.push_back(model);
            model->activated = true;
        }
        // Compute output functions and route the events.
        if (model->imminent)  // These are the imminent Mealy models
        {
            if (!model->activated) {
                model->typeIsAtomic()->output_func(*(model->outputs));
            } else {
                model->output_func(*(model->inputs), *(model->outputs));
            }
        } else {
            assert(model->activated);
            // These are the Mealy models activated by input
            model->output_func(sched.minPriority() - model->tL,
                               *(model->inputs), *(model->outputs));
        }
    }
    // Translate Mealy output to inputs for Moore models. The route method
    // will throw an exception if an event is sent to a Mealy model.
    for (auto m_iter : mealy) {
        MealyAtomic<OutputType, TimeType>* model = m_iter;
        // Route each event in y
        for (typename list<OutputType>::iterator y_iter =
                 model->outputs->begin();
             y_iter != model->outputs->end(); y_iter++) {
            route(model->getParent(), model, *y_iter);
        }
    }
    mealy.clear();
    io_up_to_date = true;
}

template <class OutputType, class TimeType>
void Simulator<OutputType, TimeType>::computeNextOutput() {
    computeNextOutput(bogus_input, sched.minPriority());
}

template <class OutputType, class TimeType>
TimeType Simulator<OutputType, TimeType>::computeNextState(
    list<Event<OutputType, TimeType>> &input, TimeType t) {
    computeNextOutput(input, t);
    assert(io_time == t && io_up_to_date);
    return computeNextState();
}

template <class OutputType, class TimeType>
TimeType Simulator<OutputType, TimeType>::computeNextState() {
    if (!io_up_to_date) {
        computeNextOutput();
    }
    io_up_to_date = false;
    TimeType t = io_time, tQ = io_time + adevs_epsilon<TimeType>();

    // Track which models need and have been called
    set<Devs<OutputType, TimeType>*> called;
    list<Devs<OutputType, TimeType>*> pending_transitions;

    /*
     * Compute the states of atomic models.  Store Network models that
     * need to have their model transition function evaluated in a
     * special container that will be used when the structure changes are
     * computed.
     */
    for (auto model : activated) {
        // Internal event if no input
        if (model->inputs->empty()) {
            model->delta_int();
        } else if (model->imminent) {
            // Confluent event if model is imminent and has input
            model->delta_conf(*(model->inputs));
        } else {
            // External event if model is not imminent and has input
            model->delta_ext(t - model->tL, *(model->inputs));
        }
        // Notify listeners
        this->notify_state_listeners(model, tQ);
        // Check for a model transition

        // Adjust position in the schedule
        schedule(model, tQ);

        bool propagate = model->model_transition();
        auto parent = model->getParent();
        if (propagate && parent != nullptr) {
            pending_transitions.push_back(parent);
        }
    }

    // The new states are in effect at t + eps so advance t
    t = tQ;

    while (!pending_transitions.empty()) {
        auto model = *(pending_transitions.begin());
        bool const already_called = called.find(model) != called.end();
        //printf("found=%d, model=%p set=%d\n", already_called, model, called.size());
        if (!already_called) {
            bool propagate = model->model_transition();
            called.insert(model);
            auto parent = model->getParent();
            if (propagate && parent != nullptr) {
                pending_transitions.push_back(parent);
            }
        }
        pending_transitions.pop_front();
    }

    // Recursive version
    // for (auto model : activated) {
    //     transition(model, called);
    // }

    // Schedule and unschedule any models that were changed
    for (auto model : pending_schedule) {
        schedule(model.get(), tQ);
    }
    pending_schedule.clear();

    for (auto model : pending_unschedule) {
        unschedule_model(model.get());
    }
    pending_unschedule.clear();

    for (auto model : activated) {
        clean_up(model);
    }

    activated.clear();
    // Return the current simulation time
    return t;
}

template <class OutputType, class TimeType>
void Simulator<OutputType, TimeType>::transition(
    Devs<OutputType, TimeType>* model,
    set<Devs<OutputType, TimeType>*> &called) {
    bool const already_called = called.find(model) != called.end();
    //printf("found=%d, model=%p set=%d\n", already_called, model, called.size());
    if (!already_called) {
        bool propagate = model->model_transition();
        called.insert(model);
        auto parent = model->getParent();
        if (propagate && parent != nullptr) {
            transition(parent, called);
        }
    }
}

template <class OutputType, class TimeType>
void Simulator<OutputType, TimeType>::clean_up(
    Devs<OutputType, TimeType>* model) {
    Atomic<OutputType, TimeType>* amodel = model->typeIsAtomic();
    if (amodel != nullptr) {
        if (!amodel->inputs->empty()) {
            amodel->inputs->clear();
        }
        if (!amodel->outputs->empty()) {

            amodel->outputs->clear();
        }
    } else {
        set<Devs<OutputType, TimeType>*> components;
        model->typeIsNetwork()->getComponents(components);
        for (auto iter : components) {
            clean_up(iter);
        }
    }
    model->activated = false;
    model->imminent = false;
}

template <class OutputType, class TimeType>
void Simulator<OutputType, TimeType>::unschedule_model(
    Devs<OutputType, TimeType>* model) {
    if (model->typeIsAtomic() != nullptr) {
        sched.schedule(model->typeIsAtomic(), adevs_inf<TimeType>());
        activated.remove(model->typeIsAtomic());
    } else {
        set<Devs<OutputType, TimeType>*> components;
        model->typeIsNetwork()->getComponents(components);
        for (auto iter : components) {
            unschedule_model(iter);
        }
    }
}

template <class OutputType, class TimeType>
void Simulator<OutputType, TimeType>::schedule(
    Devs<OutputType, TimeType>* model, TimeType t) {
    model->simulator = this;
    Atomic<OutputType, TimeType>* a = model->typeIsAtomic();
    if (a != nullptr) {
        a->tL = t;
        TimeType dt = a->ta();
        if (dt == adevs_inf<TimeType>()) {
            sched.schedule(a, adevs_inf<TimeType>());
        } else {
            TimeType tNext = a->tL + dt;
            if (tNext < a->tL) {
                exception err("Negative time advance", a);
                throw err;
            }
            sched.schedule(a, tNext);
        }
    } else {
        set<Devs<OutputType, TimeType>*> components;
        model->typeIsNetwork()->getComponents(components);
        for (auto iter : components) {
            schedule(iter, t);
        }
    }
}

template <class OutputType, class TimeType>
void Simulator<OutputType, TimeType>::inject_event(
    Atomic<OutputType, TimeType>* model, OutputType &value) {
    if (io_time < model->tL) {
        exception err("Attempt to apply input in the past", model);
        throw err;
    }
    // If this is a Mealy model, add it to the list of models that
    // will need their input calculated
    if (model->typeIsMealyAtomic()) {
        if (allow_mealy_input) {
            // Add it to the list of its not already there
            if (!model->activated && !model->imminent) {
                mealy.push_back(model->typeIsMealyAtomic());
            }
        } else {
            exception err("Mealy model coupled to a Mealy model", model);
            throw err;
        }
    }
    // Add the output to the model's list of output to be processed
    if (!model->activated) {
        if (!model->imminent) {
            activated.push_back(model);
            model->activated = true;
        }
        //model->inputs->clear();
    }
    this->notify_input_listeners(model, value, io_time);
    model->inputs->push_back(value);
}

template <class OutputType, class TimeType>
void Simulator<OutputType, TimeType>::route(
    Network<OutputType, TimeType>* parent, Devs<OutputType, TimeType>* src,
    OutputType &x) {
    // Notify event listeners if this is an output event
    if (parent != src) {
        this->notify_output_listeners(src, x, io_time);
    }
    // No one to do the routing, so return
    if (parent == nullptr) {
        return;
    }
    // Compute the set of receivers for this value
    // TODO: Does it make sense to build this object every call?
    list<Event<OutputType, TimeType>> receivers;
    parent->route(x, src, receivers);
    // Deliver the event to each of its targets
    Atomic<OutputType, TimeType>* amodel = nullptr;

    for (auto rr : receivers) {
        /*
         * If the destination is an atomic model, add the event to the IO list
         * for that model and add model to the list of activated models
         */
        amodel = rr.model->typeIsAtomic();
        if (amodel != nullptr) {
            inject_event(amodel, rr.value);
        } else if (rr.model == parent) {
            // This is an external output from the parent model
            route(parent->getParent(), parent, rr.value);
        } else {
            // This is an input to a coupled model
            this->notify_input_listeners(rr.model, rr.value, io_time);
            route(rr.model->typeIsNetwork(), rr.model, rr.value);
        }
    }
    receivers.clear();
}

template <class OutputType, class TimeType>
void Simulator<OutputType, TimeType>::getAllChildren(
    Network<OutputType, TimeType>* model, set<Devs<OutputType, TimeType>*> &s) {
    set<Devs<OutputType, TimeType>*> tmp;
    // Get the component set
    model->getComponents(tmp);
    // Add all of the local level elements to s
    s.insert(tmp.begin(), tmp.end());
    // Find the components of type network and update s recursively

    for (auto iter : tmp) {
        if (iter->typeIsNetwork() != nullptr) {
            getAllChildren(iter->typeIsNetwork(), s);
        }
    }
}

}  // namespace adevs

#endif
