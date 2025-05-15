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
 * ANY EOutputTypePRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EOutputTypeEMPLARY, OR CONSEQUENTIAL DAMAGES
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
#include "adevs/graph.h"
#include "adevs/models.h"
#include "adevs/sched.h"


namespace adevs {

/**
 * The EventListener interface is used to be notified of events as
 * they occur in a simulation. It must be registered with a Simulator.
 */
template <typename ValueType, typename TimeType = double>
class EventListener {
  public:
    virtual ~EventListener() {}
    /**
     * Called when a atomic model produces an output.
     * @param model The atomic model that produced the output
     * @param value The output value produced by the model
     * @param t The time of the output event
     */
    virtual void outputEvent(Atomic<ValueType, TimeType> &model,
                             PinValue<ValueType> &value, TimeType t) = 0;
    /**
     * Called when an atomic receives an input.
     * @param model The atomic model that receives the input
     * @param value The input value
     * @param t The time of the input event
     */
    virtual void inputEvent(Atomic<ValueType, TimeType> &model,
                            PinValue<ValueType> &value, TimeType t) = 0;
    /**
     * Called after an atomic changes its state.
     * @param model The atomic model that changed state
     * @param t The time of when the change occurred
     */
    virtual void stateChange(Atomic<ValueType, TimeType> &model,
                             TimeType t) = 0;
};

/*
 * This Simulator class implements the DEVS simulation algorithm.
 * Its methods throw adevs::exception objects if any of the DEVS model
 * constraints are violated (i.e., a negative time advance, a model
 * attempting to send an input directly to itself, or coupled Mealy
 * type systems).
 */
template <class OutputType, class TimeType = double>
class Simulator {

  public:
    /*
     * Create a simulator for the atomic model. The simulator
     * constructor will fail and throw an adevs::exception if the
     * time advance of the model is less than zero.
     * @param model The model to simulate
     */
    Simulator(shared_ptr<Atomic<OutputType, TimeType>> model);

    /*
     * Initialize the simulator with a collection of models.
     * @param model The graph to simulate. The graph is set to
     * provisional and must remain so until the simulation is
     * complete.
     */
    Simulator(std::shared_ptr<Graph<OutputType, TimeType>> model);

    /*
     * Get the model's next event time. This is the time of the
     * next output and change of state.
     * @return The absolute time of the next event
     */
    TimeType nextEventTime() { return sched.minPriority(); }

    /*
     * Execute the simulation cycle at time nextEventTime()
     * @return The current simulation time
     */
    TimeType execNextEvent() { return computeNextState(); }

    /**
     * Inject an input that will be applied at the next call to
     * computeNextState and computeNextOutput. The list is automatically
     * cleared when computeNextState is called.
     */
    void injectInput(PinValue<OutputType> &x) { external_input.push_back(x); }

    /**
     * Clear the list of injected inputs.
     */
    void clearInjectedInput() { external_input.clear(); }

    /**
     * Set the next event time to something less than the
     * nextEventTime method would return. This is used to
     * force the simulator to apply injected inputs at this
     * time.
     */
    void setNextTime(TimeType t) {
        if (t != tNext) {
            output_ready = false;
        }
        tNext = t;
    }

    /*
     * Compute the output values of the imminent component models.
     * This notifies EventListeners as the outputs are produced.
     */
    void computeNextOutput();

    /*
     * Compute the next state of the model. Provisional changes to
     * the model structure are applied after new states are computed
     * for the atomic components.
     * @return The new simulation time
     */
    TimeType computeNextState();

    /**
     * Add an event listener to the simulator that will be notified of
     * input, output, and changes of state as they occur.
     * @param listener The event listener to add
     */
    void addEventListener(
        std::shared_ptr<EventListener<OutputType, TimeType>> listener) {
        listeners.push_back(listener);
    }

  private:
    std::shared_ptr<Graph<OutputType, TimeType>> graph;
    std::list<Atomic<OutputType, TimeType>*> imm;
    std::list<shared_ptr<EventListener<OutputType, TimeType>>> listeners;
    std::list<PinValue<OutputType>> external_input;
    Schedule<OutputType, TimeType> sched;
    bool output_ready;
    TimeType tNext;

    void schedule(Atomic<OutputType, TimeType>* model, TimeType t);
};

template <typename OutputType, typename TimeType>
Simulator<OutputType, TimeType>::Simulator(
    std::shared_ptr<Graph<OutputType, TimeType>> model)
    : graph(model), output_ready(false) {
    graph->set_provisional(true);
    for (auto atomic : model->get_atomics()) {
        schedule(atomic.get(), adevs_zero<TimeType>());
    }
    tNext = sched.minPriority();
}

template <typename OutputType, typename TimeType>
Simulator<OutputType, TimeType>::Simulator(
    std::shared_ptr<Atomic<OutputType, TimeType>> model)
    : graph(new Graph<OutputType, TimeType>()), output_ready(false) {
    graph->add_atomic(model);
    graph->set_provisional(true);
    schedule(model.get(), adevs_zero<TimeType>());
    tNext = sched.minPriority();
}

template <class OutputType, class TimeType>
void Simulator<OutputType, TimeType>::computeNextOutput() {
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
                listener->outputEvent(*model, y, tNext);
            }
        }
    }
}

template <class OutputType, class TimeType>
TimeType Simulator<OutputType, TimeType>::computeNextState() {
    PinValue<OutputType> x;
    TimeType t = tNext + adevs_epsilon<TimeType>();
    if (!output_ready) {
        computeNextOutput();
    }
    output_ready = false;
    std::set<Atomic<OutputType, TimeType>*> active;
    std::list<std::pair<pin_t, std::shared_ptr<Atomic<OutputType, TimeType>>>> input;
    /// Construct input bags for each model and get the active set
    for (auto producer : imm) {
        active.insert(producer);
        for (auto y : producer->outputs) {
            x.value = y.value;
            graph->route(y.pin, input);
            for (auto consumer : input) {
                active.insert(consumer.second.get());
                x.pin = consumer.first;
                consumer.second->inputs.push_back(x);
                for (auto listener : listeners) {
                    listener->inputEvent(*(consumer.second.get()), x, tNext);
                }
            }
            input.clear();
        }
        producer->outputs.clear();
    }
    for (auto y : external_input) {
        x.value = y.value;
        graph->route(y.pin, input);
        for (auto consumer : input) {
            active.insert(consumer.second.get());
            x.pin = consumer.first;
            consumer.second->inputs.push_back(x);
            for (auto listener : listeners) {
                listener->inputEvent(*(consumer.second.get()), x, tNext);
            }
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
        for (auto listener : listeners) {
            listener->stateChange(*model, tNext);
        }
        // Adjust position in the schedule
        schedule(model, t);
    }
    // Effect any changes in the model structure
    graph->set_provisional(false);
    std::list<typename Graph<OutputType,TimeType>::graph_op>& pending = graph->get_pending();
    while (!pending.empty()) {
        auto op = pending.front();
        pending.pop_front();
        switch(op.op)
        {
            case Graph<OutputType, TimeType>::ADD_ATOMIC:
                graph->add_atomic(op.model);
                schedule(op.model.get(), t);
                break;
            case Graph<OutputType, TimeType>::REMOVE_ATOMIC:
                sched.schedule(op.model.get(), adevs_inf<TimeType>());
                graph->remove_atomic(op.model);
                break;
            case Graph<OutputType, TimeType>::REMOVE_PIN:
                graph->remove_pin(op.pin[0]);
                break;
            case Graph<OutputType, TimeType>::CONNECT_PIN_TO_PIN:
                graph->connect(op.pin[0], op.pin[1]);
                break;
            case Graph<OutputType, TimeType>::DISCONNECT_PIN_FROM_PIN:
                graph->disconnect(op.pin[0], op.pin[1]);
                break;
            case Graph<OutputType, TimeType>::CONNECT_PIN_TO_ATOMIC:
                graph->connect(op.pin[0], op.model);
                break;
            case Graph<OutputType, TimeType>::DISCONNECT_PIN_FROM_ATOMIC:
                graph->disconnect(op.pin[0], op.model);
                break;
        }
    }
    graph->set_provisional(true);
    // Get the time of next event and return
    tNext = sched.minPriority();
    return t;
}

template <class OutputType, class TimeType>
void Simulator<OutputType, TimeType>::schedule(
    Atomic<OutputType, TimeType>* model, TimeType t) {
    model->tL = t;
    TimeType dt = model->ta();
    if (dt == adevs_inf<TimeType>()) {
        model->tN = adevs_inf<TimeType>();
        sched.schedule(model, adevs_inf<TimeType>());
    } else {
        model->tN = model->tL + dt;
        if (dt < adevs_zero<TimeType>()) {
            exception err("Negative time advance", model);
            throw err;
        }
        sched.schedule(model, model->tN);
    }
}

}  // namespace adevs

#endif
