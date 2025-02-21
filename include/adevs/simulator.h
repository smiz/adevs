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
#include "adevs/models.h"
#include "adevs/sched.h"
#include "adevs/graph.h"

namespace adevs {

template <typename ValueType, typename TimeType = double>
class EventListener {
    public:
        virtual ~EventListener(){}
        virtual void outputEvent(
            Atomic<ValueType,TimeType>& model, PinValue<ValueType>& value, TimeType t) = 0;
};

/*
 * This Simulator class implements the DEVS simulation algorithm.
 * Its methods throw adevs::exception objects if any of the DEVS model
 * constraints are violated (i.e., a negative time advance, a model
 * attempting to send an input directly to itself, or coupled Mealy
 * type systems).
 */
template <class X, class T = double>
class Simulator {

  public:
    /*
     * Create a simulator for the atomic model. The simulator
     * constructor will fail and throw an adevs::exception if the
     * time advance of the model is less than zero.
     * @param model The model to simulate
     */
    Simulator(shared_ptr<Atomic<X, T>>& model);

    /*
     * Initialize the simulator with a collection of models.
     * @param model The collection of models to simulate.
     */
    Simulator(std::shared_ptr<Graph<X, T>>& model);

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

    /**
     * Inject an input that will be applied at the next calls to
     * computeNextState and computeNextOutput. The list is automatically
     * cleared when computeNextState is called.
     */
    void injectInput(PinValue<X>& x) {
        external_input.push_back(x);
    }
    /**
     * Clear the list of injected inputs.
     */
    void clearInjectedInput() {
        external_input.clear();
    }
    /**
     * Set the next event time to something less than the
     * nextEventTime method would return. This is used to
     * force the simulator to apply injected inputs at this
     * time.
     */
    void setNextTime(T t) {
        tNext = t;
        output_ready = false;
    }
    /*
     * Compute the output values of the imminent component models.
     * This notifies EventListeners as the outputs are produced.
     */
    void computeNextOutput();
    /*
     * Compute the next state of the model.
     * @return The new, current simulation time
     */
    T computeNextState();

    void addEventListener(std::shared_ptr<EventListener<X, T>>& listener) {
        listeners.push_back(listener);
    }

  private:
    std::shared_ptr<Graph<X,T>> graph;
    std::list<shared_ptr<Atomic<X, T>>> imm;
    std::list<shared_ptr<EventListener<X, T>>> listeners;
    std::list<PinValue<X>> external_input;
    Schedule<X, T> sched;
    bool output_ready;
    T tNext;

    void schedule(std::shared_ptr<Atomic<X, T>>& model, T t);

};

template <typename X, typename T>
Simulator<X, T>::Simulator(std::shared_ptr<Graph<X, T>> &model)
    : graph(model),output_ready(false) {
    // The Atomic constructor sets the atomic model's
    // tL correctly to zero, and so it is sufficient
    // to only worry about putting models with a
    // non infinite time advance into the schedule.
    for (auto atomic : model->get_atomics()) {
        schedule(atomic, adevs_zero<T>());
    }
    tNext = sched.minPriority();
}

template <typename X, typename T>
Simulator<X, T>::Simulator(std::shared_ptr<Atomic<X, T>> &model)
    : graph(new Graph<X,T>()),output_ready(false) {
    graph->add_atomic(model);
    schedule(model, adevs_zero<T>());
    tNext = sched.minPriority();
}

template <class X, class T>
void Simulator<X, T>::computeNextOutput() {
    output_ready = true;
    for (auto model : imm) {
        model->outputs.clear();
    }
    imm.clear();
    if (sched.minPriority() > tNext) {
        return;
    }
    imm = sched.visitImminent();
    for (auto model : imm) {
        model->output_func(model->outputs);
        for (auto listener : listeners) {
            for (auto y : model->outputs) {
                listener->outputEvent(*(model.get()), y, tNext);
            }
        }
    }
}

template <class X, class T>
T Simulator<X, T>::computeNextState() {
    T t = tNext;
    if (!output_ready) {
        computeNextOutput();
    }
    output_ready = false;
    std::set<std::shared_ptr<Atomic<X,T>>> active;
    std::list<std::shared_ptr<Atomic<X,T>>> input;
    /// Construct input bags for each model and get the active set
    for (auto producer : imm) {
        active.insert(producer);
        for (auto x : producer->outputs) {
            graph->get_atomics(x.pin,input);
            for (auto consumer : input) {
                active.insert(consumer);
                consumer->inputs.push_back(x);
            }
            input.clear();
        }
        producer->outputs.clear();
    }
    for (auto x : external_input) {
        graph->get_atomics(x.pin,input);
        for (auto consumer : input) {
             active.insert(consumer);
            consumer->inputs.push_back(x);
        }
        input.clear();
    }
    external_input.clear();
    imm.clear();
    for (auto model : active) {
        // Internal event if no input
        if (model->inputs.empty()) {
            model->delta_int();
        } else if (model->tN == tNext) {
            // Confluent event if model is imminent and has input
            model->delta_conf(model->inputs);
            model->inputs.clear();
        } else {
            // External event if model is not imminent and has input
            model->delta_ext(tNext - model->tL, model->inputs);
            model->inputs.clear();
        }
        // Adjust position in the schedule
        schedule(model, tNext);
    }
    t += adevs_epsilon<T>();
    tNext = sched.minPriority();
    return t;
}

template <class X, class T>
void Simulator<X, T>::schedule(std::shared_ptr<Atomic<X, T>>& model, T t) {
    model->tL = t;
    T dt = model->ta();
    if (dt == adevs_inf<T>()) {
        model->tN = adevs_inf<T>();
        sched.schedule(model, adevs_inf<T>());
    } else {
        model->tN = model->tL + dt;
        if (model->tN < model->tL) {
            exception err("Negative time advance", model.get());
            throw err;
        }
        sched.schedule(model, model->tN);
    }
}

}  // namespace adevs

#endif
