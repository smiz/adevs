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
 * @brief An interface for receiving notifications of state changes,
 * input events, and output events from a running simulation.
 * 
 * The EventListener interface is used to be notified of events as
 * they occur in a simulation. It must be registered with the Simulator
 * that will provide the notifications.
 */
template <typename ValueType, typename TimeType = double>
class EventListener {
  public:
    /**
     * @brief The virtual default constructor.
     * 
     * The default constructor is empty and does not perform any
     * initialization.
     */
    virtual ~EventListener() {}
    /**
     * @brief Called when a Atomic model produces an output.
     * 
     * This method is called for each PinValue appearing in the
     * list of outputs produced by the Atomic model's output_func()
     * method.
     * 
     * @param model The Atomic model that produced the output
     * @param value The PinValue created by the model
     * @param t The time of the output event
     */
    virtual void outputEvent(Atomic<ValueType, TimeType> &model,
                             PinValue<ValueType> &value, TimeType t) = 0;
    /**
     * @brief Called when an Atomic receives an input.
     * 
     * This method is called for each PinValue that is passed to the
     * Atomic model's delta_ext() and delta_conf() methods.
     * 
     * @param model The Atomic model that receives the input
     * @param value The PinValue provided as input
     * @param t The time of the input event
     */
    virtual void inputEvent(Atomic<ValueType, TimeType> &model,
                            PinValue<ValueType> &value, TimeType t) = 0;
    /**
     * @brief Called after an Atomic model changes its state.
     * 
     * This method is called after the Atomic model's delta_int(),
     * delta_ext(), and delta_conf() methods are called.
     * 
     * @param model The Atomic model that changed state
     * @param t The time when the change occurred
     */
    virtual void stateChange(Atomic<ValueType, TimeType> &model,
                             TimeType t) = 0;
};

/**
 * @brief Implements the DEVS simulation algorithm.
 *  
 * This Simulator class implements the DEVS simulation algorithm.
 * Its methods throw adevs::exception objects if any of the DEVS model
 * constraints are violated (e.g., a negative time advance). The Simulation
 * algorithm can be described briefly as follows:
 * 1. Each Atomic model has an associated clock called its elapsed time, denoted by e.
 * This value is initialized to zero. The current simulation time is set to zero.
 * 2. Call the ta() method of each Atomic model and find the smallest value of ta()-e.
 * The set of models whose ta()-e equals this smallest value are the imminent models.
 * If the smallest ta()-e is infinity, then the simulation end. For brevity, we use
 * dt to indicate this smallest ta()-e.
 * 3. Add dt to the elapsed time e of each Atomic model and to the current simulation
 * time. Notice that imminent models now have e = ta() and non-imminent models
 * have e < ta().
 * 4. Call the output_func() method of each imminent model. For each PinValue
 * object in the output list, using the Graph route() method to get the
 * the Atomic models that will receive the value as input. Put these Atomic
 * models into the active set.
 * 5. Calculate new states for the Atomic models.
 *     - If the model is imminent and is not in the active set, call its delta_int() method
 * and set its elapsed time e to zero. 
 *     - If the model is imminent and is in the active set, call its delta_conf() method
 * with the input produced in step 4. Set its elapsed time e to zero.         
 *     - If the model is not imminent and is in the active set, call its delta_ext() method
 * with its elapsed time e and the input produced in step 4. Set its elapsed time e to zero.
 *     - Otherwise, do nothing.
 * 6. Apply provisional changes to the Graph structure.
 * 7. Go to step 2.
 * 
 * Looking at this algorithm we can see two important features. First, the time advance ta()
 * of each model is used to determine the time of the next event. The value returned by time advance 
 * is how long the Atomic model will stay in its current state before changing state spontaneously.
 * Second, the current simulation time is the sum of the dt values calculated in each iteration
 * of the algorithm.
 * 
 * The Simulator is designed to be used as a component within a larger simulation.
 * To illustrate how the Simulator can be used in this way, let us sketch its use to
 * create a federate as described in the IEEE 1516 (High Level Architecture) standard.
 * The general idea is as follows:
 * 1. Register one or more EventListener objects that will gather output events that are to
 * be published to the HLA federation.
 * 2. Get your time of next event by calling the Simulator nextEventTime() method.
 * 3. Call computeNextOutput(). Send Events and Object Updates (i.e., HLA messages)
 * as your registered EventListener objects receive output events. The time stamp for
 * these messages is the time returned by nextEventTime(). Make sure they are published
 * as retractable messages so that they can be canceled if you receive input before
 * your next event time.
 * 4. Issued a next event request to the HLA federation for your nextEventTime().
 * 5. You will receive one or more messages from the federation, each with an identical
 * time stamp.
 *   -# If this time stamp is less than your nextEventTime() then call setNextTime()
 * with the time stamp of the received message and retract any messages published in
 * step 3.
 *   -# Inject the data from the received messages into your Simulator by using the injectInput() method.
 * 6. When the time advance grant is received from the federation, call computeNextState()
 * and repeat from step 2.
 */
template <class OutputType, class TimeType = double>
class Simulator {

  public:
    /** 
     * @brief Create a simulator for the atomic model.
     * 
     * The constructor will fail and throw an adevs::exception if the
     * time advance of the model is less than zero.
     * @param model The model to simulate.
     */
    Simulator(shared_ptr<Atomic<OutputType, TimeType>> model);

    /**
     * @brief Initialize the simulator with a collection of models.
     
     * The constructor will fail and throw an adevs::exception if the
     * time advance of the model is less than zero.
     * @param model The graph to simulate.
     */
    Simulator(std::shared_ptr<Graph<OutputType, TimeType>> model);

    /**
     * @brief Get the time of the next event.
     
     * This is the absolute time of the next output and change of state.
     * It is the time that will be assigned to the current time in
     * step 3 of the simulation algorithm.
     * 
     * @return The absolute time of the next event.
     */
    TimeType nextEventTime() { return tNext; }

    /** 
     * @brief Execute the simulation cycle at time nextEventTime().
     * 
     * Update the simulation time to match nextEventTime() and calculate
     * the output and next states as described in steps 4, 5, and 6 of the
     * simulation algorithm.
     * 
     * @return The current simulation time
     */
    TimeType execNextEvent() { return computeNextState(); }

    /**
     * @brief Inject an event into the simulation.
     * 
     * Inject an input that will be applied at the next call to
     * computeNextState() and computeNextOutput(). The event is routed to
     * each model that is reachable from the pin of the injected PinValue
     * object.
     * 
     * @param x The PinValue to inject into the simulation.
     */
    void injectInput(PinValue<OutputType> &x) { external_input.push_back(x); }

    /**
     * @brief Clear the list of injected inputs.
     * 
     * This erases all injected inputs that have not yet been applied
     * to the simulation via a call to computeNextState() or computeNextOutput().
     * It is cleared automatically at each call to computeNextState(),
     */
    void clearInjectedInput() { external_input.clear(); }

    /**
     * @brief Set the next event time to something less than the
     * nextEventTime() method would return.
     * 
     * This is used to force the simulator to apply injected inputs at 
     * supplied time
     * 
     * @param t The time at which the next event will occur.
     */
    void setNextTime(TimeType t) {
        if (t != tNext) {
            output_ready = false;
        }
        tNext = t;
    }

    /**
     * @brief Compute the output values of the imminent component models.
     * 
     * This notifies EventListener as the outputs are produced. It
     * does not change the simulation time or states of the models.
     */
    void computeNextOutput();

    /**
     * @brief Compute the next state of the model.
     * 
     * This notifies EventListener as inputs are applied to models
     * and as new states are calculated. Provisional changes to
     * the model structure are applied after new states are computed
     * for the Atomic components.
     * 
     * @return The current simulation time.
     */
    TimeType computeNextState();

    /**
     * @brief Register an EventListener with the Simulator.
     * 
     * Add an event listener to the simulator that will be notified of
     * input, output, and changes of state as they occur.
     * 
     * @param listener The EventListener to be added.
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
