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
#ifndef __adevs_simulator_h_
#define __adevs_simulator_h_
#include <cassert>
#include <cstdlib>
#include <list>
#include <set>
#include "adevs/abstract_simulator.h"
#include "adevs/bag.h"
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
template <class X, class T = double>
class Simulator : public AbstractSimulator<X, T>,
                  private Schedule<X, T>::ImminentVisitor {

  public:
    /*
     * Create a simulator for a model. The simulator
     * constructor will fail and throw an adevs::exception if the
     * time advance of any component atomic model is less than zero.
     * @param model The model to simulate
     */
    Simulator(shared_ptr<Devs<X, T>> model)
        : AbstractSimulator<X, T>(),
          Schedule<X, T>::ImminentVisitor(),
          io_up_to_date(false) {
        schedule(model.get(), adevs_zero<T>());
    }

    /*
     * Initialize the simulator with a list of models
     * that will be active at the start. The constructor
     * only schedules the models in the list, which avoids
     * the overhead of constructing the full set of
     * models with recursive calls to Network::getComponents().
     * @param active The list of active models.
     */
    Simulator(std::list<shared_ptr<Devs<X, T>>> &active);

    /*
     * Get the model's next event time
     * @return The absolute time of the next event
     */
    T nextEventTime() { return sched.minPriority(); }

    /*
     * Execute the simulation cycle at time nextEventTime()
     * @return The updated simulation time
     */
    T execNextEvent() { return computeNextState(); }

    /*
     * Execute until nextEventTime() > tend.
     * @return The updated simulation time
     */
    T execUntil(T tend) {
        T t = tend + adevs_epsilon<T>();
        while (nextEventTime() <= tend && nextEventTime() < adevs_inf<T>()) {
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
     * @param input A bag of (input target,value) pairs
     * @param t The time at which the input takes effect
     */
    void computeNextOutput(Bag<Event<X, T>> &input, T t);

    /*
     * Apply the bag of inputs at time t and then compute the next model
     * states. Requires that lastEventTime() <= t <= nextEventTime().
     * This, in effect, implements the state transition function of
     * the resultant model. If the output has already been computed
     * at time t, then the new input at t is simply appended to the
     * prior input. Otherwise, the old results are discarded and input
     * is calculated at the given time.
     * @param input A bag of (input target,value) pairs
     * @param t The time at which the input takes effect
     * @return The new, current simulation time
     */

    T computeNextState(Bag<Event<X, T>> &input, T t);

    /*
     * Compute the next state at the time at the time t and with
     * input supplied at the prior call to computeNextOutput
     * assuming no computeNextState has intervened. Assumes
     * t = nextEventTime() and input an empty bag if there was
     * no prior call to computeNextOutput.
     * @return The new, current simulation time
     */
    T computeNextState();

    /*
     * Deletes the simulator, but leaves the model intact. The model must
     * exist when the simulator is deleted.  Delete the model only after
     * the Simulator is deleted.
     */
    ~Simulator();

    /*
     * Assign a model to the simulator. This has the same effect as passing
     * the model to the constructor.
     */
    void addModel(shared_ptr<Atomic<X, T>> model) {
        schedule(model.get(), adevs_zero<T>());
    }

  private:
    // Bogus input bag for execNextEvent() method
    Bag<Event<X, T>> bogus_input;
    // The event schedule
    Schedule<X, T> sched;
    // List of models that are imminent or activated by input
    Bag<Atomic<X, T>*> activated;
    // Mealy systems that we need to process
    bool allow_mealy_input;
    bool io_up_to_date;

    T io_time;
    Bag<MealyAtomic<X, T>*> mealy;

    /*
     * Recursively add the model and its elements to the schedule
     * using t as the time of last event.
     */
    void schedule(Devs<X, T>* model, T t);

    /// Route an event generated by the source model contained in the parent model.
    void route(Network<X, T>* parent, Devs<X, T>* src, X &x);

    /*
     * Add an input to the input bag of an an atomic model. If the
     * model is not already active , then this method adds the model to
     * the activated bag.
     */
    void inject_event(Atomic<X, T>* model, X &value);

    /*
     * Recursively remove a model and its components from the schedule
     * and the imminent/activated bags
     */
    void unschedule_model(Devs<X, T>* model);

    /*
     * Delete any thing in the output bag, and return the input
     * and output bags to the pools.
     * Recursively clean up network model components.
     */
    void clean_up(Devs<X, T>* model);

    /*
     * Construct the complete descendant set of a network model and store it in s.
     */
    void getAllChildren(Network<X, T>* model, set<Devs<X, T>*> &s);

    /*
     * Visit method inhereted from ImminentVisitor
     */
    void visit(Atomic<X, T>* model);
};

template <typename X, typename T>
Simulator<X, T>::Simulator(std::list<shared_ptr<Devs<X, T>>> &active)
    : AbstractSimulator<X, T>(),
      Schedule<X, T>::ImminentVisitor(),
      io_up_to_date(false) {
    // The Atomic constructor sets the atomic model's
    // tL correctly to zero, and so it is sufficient
    // to only worry about putting models with a
    // non infinite time advance into the schedule.
    for (auto iter : active) {
        schedule(iter.get(), adevs_zero<T>());
    }
}

template <class X, class T>
void Simulator<X, T>::visit(Atomic<X, T>* model) {
    assert(model->outputs->empty());
    model->imminent = true;
    // Mealy models are processed after the Moore models
    if (model->typeIsMealyAtomic() != NULL) {
        assert(model->outputs->empty());
        // May be in the mealy list because of a route call
        if (!model->activated) {
            mealy.push_back(model->typeIsMealyAtomic());
        }
        return;
    }
    //model->outputs->clear();

    // Put it in the active list if it is not already there
    if (!model->activated) {
        activated.push_back(model);
        model->activated = true;
    }

    // Compute output functions and route the events.
    model->output_func(*(model->outputs));
    // Route each event in y
    for (typename Bag<X>::iterator y_iter = model->outputs->begin();
         y_iter != model->outputs->end(); y_iter++) {
        route(model->getParent(), model, *y_iter);
    }
}

template <class X, class T>
void Simulator<X, T>::computeNextOutput(Bag<Event<X, T>> &input, T t) {
    // Undo any prior output calculation at another time
    if (io_up_to_date && !(io_time == t)) {
        typename Bag<Atomic<X, T>*>::iterator iter;
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
        sched.visitImminent(this);
    }
    // Apply the injected inputs.
    for (auto iter : input) {
        Atomic<X, T>* amodel = iter.model->typeIsAtomic();
        if (amodel != NULL) {
            inject_event(amodel, iter.value);
        } else {
            route(iter.model->typeIsNetwork(), iter.model, iter.value);
        }
    }
    // Only Moore models can influence Mealy models.
    allow_mealy_input = false;
    // Iterate over activated Mealy models to calculate their output
    for (auto m_iter : mealy) {
        MealyAtomic<X, T>* model = m_iter;
        assert(model->outputs->empty());
        //model->outputs->clear();

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
        MealyAtomic<X, T>* model = m_iter;
        // Route each event in y
        for (typename Bag<X>::iterator y_iter = model->outputs->begin();
             y_iter != model->outputs->end(); y_iter++) {
            route(model->getParent(), model, *y_iter);
        }
    }
    mealy.clear();
    io_up_to_date = true;
}

template <class X, class T>
void Simulator<X, T>::computeNextOutput() {
    computeNextOutput(bogus_input, sched.minPriority());
}

template <class X, class T>
T Simulator<X, T>::computeNextState(Bag<Event<X, T>> &input, T t) {
    computeNextOutput(input, t);
    assert(io_time == t && io_up_to_date);
    return computeNextState();
}

template <class X, class T>
T Simulator<X, T>::computeNextState() {
    if (!io_up_to_date) {
        computeNextOutput();
    }
    io_up_to_date = false;
    T t = io_time, tQ = io_time + adevs_epsilon<T>();
    /*
     * Compute the states of atomic models.  Store Network models that
     * need to have their model transition function evaluated in a
     * special container that will be used when the structure changes are
     * computed.
     */
    for (auto model : activated) {
        //Atomic<X, T>* model = activated[i];
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
        clean_up(model);
    }

    // The new states are in effect at t + eps so advance t
    t = tQ;
    activated.clear();
    // Return the current simulation time
    return t;
}

template <class X, class T>
void Simulator<X, T>::clean_up(Devs<X, T>* model) {
    Atomic<X, T>* amodel = model->typeIsAtomic();
    if (amodel != NULL) {
        if (!amodel->inputs->empty()) {
            amodel->inputs->clear();
        }
        if (!amodel->outputs->empty()) {

            amodel->outputs->clear();
        }
    } else {
        set<Devs<X, T>*> components;
        model->typeIsNetwork()->getComponents(components);
        for (auto iter : components) {
            clean_up(iter);
        }
    }
    model->activated = false;
    model->imminent = false;
}

template <class X, class T>
void Simulator<X, T>::unschedule_model(Devs<X, T>* model) {
    if (model->typeIsAtomic() != NULL) {
        sched.schedule(model->typeIsAtomic(), adevs_inf<T>());
        activated.remove(model->typeIsAtomic());
    } else {
        set<Devs<X, T>*> components;
        model->typeIsNetwork()->getComponents(components);
        for (auto iter : components) {
            unschedule_model(iter);
        }
    }
}

template <class X, class T>
void Simulator<X, T>::schedule(Devs<X, T>* model, T t) {
    Atomic<X, T>* a = model->typeIsAtomic();
    if (a != NULL) {
        a->tL = t;
        T dt = a->ta();
        if (dt == adevs_inf<T>()) {
            sched.schedule(a, adevs_inf<T>());
        } else {
            T tNext = a->tL + dt;
            if (tNext < a->tL) {
                exception err("Negative time advance", a);
                throw err;
            }
            sched.schedule(a, tNext);
        }
    } else {
        set<Devs<X, T>*> components;
        model->typeIsNetwork()->getComponents(components);
        for (auto iter : components) {
            schedule(iter, t);
        }
    }
}

template <class X, class T>
void Simulator<X, T>::inject_event(Atomic<X, T>* model, X &value) {
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
    // Add the output to the model's bag of output to be processed
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

template <class X, class T>
void Simulator<X, T>::route(Network<X, T>* parent, Devs<X, T>* src, X &x) {
    // Notify event listeners if this is an output event
    if (parent != src) {
        this->notify_output_listeners(src, x, io_time);
    }
    // No one to do the routing, so return
    if (parent == NULL) {
        return;
    }
    // Compute the set of receivers for this value
    // TODO: Does it make sense to build this object every call?
    Bag<Event<X, T>> receivers;
    parent->route(x, src, receivers);
    // Deliver the event to each of its targets
    Atomic<X, T>* amodel = NULL;

    for (auto rr : receivers) {
        /*
         * If the destination is an atomic model, add the event to the IO bag
         * for that model and add model to the list of activated models
         */
        amodel = rr.model->typeIsAtomic();
        if (amodel != NULL) {
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

template <class X, class T>
void Simulator<X, T>::getAllChildren(Network<X, T>* model,
                                     set<Devs<X, T>*> &s) {
    set<Devs<X, T>*> tmp;
    // Get the component set
    model->getComponents(tmp);
    // Add all of the local level elements to s
    s.insert(tmp.begin(), tmp.end());
    // Find the components of type network and update s recursively

    for (auto iter : tmp) {
        if (iter->typeIsNetwork() != NULL) {
            getAllChildren(iter->typeIsNetwork(), s);
        }
    }
}

template <class X, class T>
Simulator<X, T>::~Simulator() {
    // Clean up the models with stale IO
    for (auto iter : activated) {
        clean_up(iter);
    }
}

}  // namespace adevs

#endif
