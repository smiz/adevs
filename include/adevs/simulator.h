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

template <typename ValueType, typename TimeType = double>
class EventListener {
  public:
    virtual ~EventListener() {}
    virtual void outputEvent(Atomic<ValueType, TimeType> &model,
                             PinValue<ValueType> &value, TimeType t) = 0;
    virtual void inputEvent(Atomic<ValueType, TimeType> &model,
                            PinValue<ValueType> &value, TimeType t) = 0;
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
    // : public AbstractSimulator<OutputType, TimeType> {

  public:
    /*
     * Create a simulator for the atomic model. The simulator
     * constructor will fail and throw an adevs::exception if the
     * time advance of the model is less than zero.
     * @param model The model to simulate
     */
    Simulator(shared_ptr<Atomic<OutputType, TimeType>> model);
    // : AbstractSimulator<OutputType, TimeType>(), io_up_to_date(false) {

    /*
     * Initialize the simulator with a collection of models.
     * @param model The collection of models to simulate.
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
     * Inject an input that will be applied at the next calls to
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

    // TimeType execUntil(TimeType tend) {
    //     TimeType t = tend + adevs_epsilon<TimeType>();
    //     while (nextEventTime() <= tend &&
    //            nextEventTime() < adevs_inf<TimeType>()) {
    //         t = execNextEvent();
    //     }
    //     return t;
    // }

    /*
     * Compute the output values of the imminent component models.
     * This notifies EventListeners as the outputs are produced.
     */
    void computeNextOutput();

    // /*
    //  * Compute the output value of the model in response to an input
    //  * at some time in lastEventTime() <= t <= nextEventTime().
    //  * This will notify registered EventListeners as the outputs
    //  * are produced. If this is the first call since the prior
    //  * state change with the given t, then the new output is computed.
    //  * Subsequent calls for the same time t simply
    //  * append to the input already supplied at time t.
    //  * @param input A list of (input target,value) pairs
    //  * @param t The time at which the input takes effect
    //  */
    // void computeNextOutput(list<Event<OutputType, TimeType>> &input,
    //                        TimeType t);

    // /*
    //  * Apply the list of inputs at time t and then compute the next model
    //  * states. Requires that lastEventTime() <= t <= nextEventTime().
    //  * This, in effect, implements the state transition function of
    //  * the resultant model. If the output has already been computed
    //  * at time t, then the new input at t is simply appended to the
    //  * prior input. Otherwise, the old results are discarded and input
    //  * is calculated at the given time.
    //  * @param input A list of (input target,value) pairs
    //  * @param t The time at which the input takes effect
    //  * @return The new, current simulation time
    //  */
    // TimeType computeNextState(list<Event<OutputType, TimeType>> &input,
    //                           TimeType t);

    // /*
    //  * Compute the next state at the time at the time t and with
    //  * input supplied at the prior call to computeNextOutput
    //  * assuming no computeNextState has intervened. Assumes
    //  * t = nextEventTime() and input an empty list if there was
    //  * no prior call to computeNextOutput.
    //  */
    /*
     * Compute the next state of the model.
     * @return The new, current simulation time
     */
    TimeType computeNextState();

    void addEventListener(
        std::shared_ptr<EventListener<OutputType, TimeType>> listener) {
        listeners.push_back(listener);
    }

  private:
    std::shared_ptr<Graph<OutputType, TimeType>> graph;
    std::list<shared_ptr<Atomic<OutputType, TimeType>>> imm;
    std::list<shared_ptr<EventListener<OutputType, TimeType>>> listeners;
    std::list<PinValue<OutputType>> external_input;
    Schedule<OutputType, TimeType> sched;
    bool output_ready;
    //bool allow_mealy_input;
    //bool io_up_to_date;
    TimeType tNext;
    //TimeType io_time;
    //list<Atomic<OutputType, TimeType>*> activated;
    //list<MealyAtomic<OutputType, TimeType>*> mealy;

    void schedule(std::shared_ptr<Atomic<OutputType, TimeType>> &model,
                  TimeType t);
};

template <typename OutputType, typename TimeType>
Simulator<OutputType, TimeType>::Simulator(
    std::shared_ptr<Graph<OutputType, TimeType>> model)
    : graph(model), output_ready(false) {
    //: AbstractSimulator<OutputType, TimeType>(), io_up_to_date(false) {
    // The Atomic constructor sets the atomic model's
    // tL correctly to zero, and so it is sufficient
    // to only worry about putting models with a
    // non infinite time advance into the schedule.
    for (auto atomic : model->get_atomics()) {
        schedule(atomic, adevs_zero<TimeType>());
    }
    tNext = sched.minPriority();
}

template <typename OutputType, typename TimeType>
Simulator<OutputType, TimeType>::Simulator(
    std::shared_ptr<Atomic<OutputType, TimeType>> model)
    : graph(new Graph<OutputType, TimeType>()), output_ready(false) {
    graph->add_atomic(model);
    schedule(model, adevs_zero<TimeType>());
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
                listener->outputEvent(*(model.get()), y, tNext);
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
    std::set<std::shared_ptr<Atomic<OutputType, TimeType>>> active;
    std::list<std::pair<pin_t, std::shared_ptr<Atomic<OutputType, TimeType>>>>
        input;
    /// Construct input bags for each model and get the active set
    for (auto producer : imm) {
        active.insert(producer);
        for (auto y : producer->outputs) {
            x.value = y.value;
            graph->route(y.pin, input);
            for (auto consumer : input) {
                active.insert(consumer.second);
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
            active.insert(consumer.second);
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
            listener->stateChange(*(model.get()), tNext);
        }
        // Adjust position in the schedule
        schedule(model, t);
    }
    tNext = sched.minPriority();
    return t;
}

template <class OutputType, class TimeType>
void Simulator<OutputType, TimeType>::schedule(
    std::shared_ptr<Atomic<OutputType, TimeType>> &model, TimeType t) {
    model->tL = t;
    TimeType dt = model->ta();
    if (dt == adevs_inf<TimeType>()) {
        model->tN = adevs_inf<TimeType>();
        sched.schedule(model, adevs_inf<TimeType>());
    } else {
        model->tN = model->tL + dt;
        if (dt < adevs_zero<TimeType>()) {
            exception err("Negative time advance", model.get());
            throw err;
        }
        sched.schedule(model, model->tN);
    }
}

//     template <class OutputType, class TimeType>
//     void Simulator<OutputType, TimeType>::computeNextOutput(
//         list<Event<OutputType, TimeType>> & input, TimeType t) {

//     // Undo any prior output calculation at another time
//     if (io_up_to_date && !(io_time == t)) {
//         typename list<Atomic<OutputType, TimeType>*>::iterator
//             iter;
//         for (auto iter : activated) {
//             clean_up(iter);
//         }
//         activated.clear();
//     }
//     // Input and output happen at the current time.
//     io_time = t;
//     // Get the imminent Moore models from the schedule if we have not
//     // already done so.
//     allow_mealy_input = true;
//     if (t == sched.minPriority() && !io_up_to_date) {
//         // get the list of activated models
//         auto activated_models = sched.visitImminent();

//         // process moore models
//         for (auto model : activated_models) {
//             Atomic<OutputType, TimeType>* atmoic_model =
//                 model->typeIsAtomic();
//             MealyAtomic<OutputType, TimeType>* mealy_model =
//                 model->typeIsMealyAtomic();

//             // need to do this because a MealyAtomic is technically both an
//             // Atomic and a MealyAtomic (by inheritance)
//             if (atmoic_model != nullptr &&
//                 mealy_model == nullptr) {
//                 assert(model->outputs->empty());
//                 activated.push_back(model);
//                 model->output_func(*(model->outputs));

//                 for (auto y_iter : *(model->outputs)) {
//                     route(model->getParent(), model, y_iter);
//                 }
//             } else {
//                 assert(mealy_model != nullptr);
//                 assert(mealy_model->outputs->empty());
//                 mealy.push_back(mealy_model);
//             }
//         }
//     }
//     // Apply the injected inputs.
//     for (auto iter : input) {
//         Atomic<OutputType, TimeType>* amodel =
//             iter.model->typeIsAtomic();
//         if (amodel != nullptr) {
//             inject_event(amodel, iter.value);
//         } else {
//             route(iter.model->typeIsNetwork(), iter.model,
//                   iter.value);
//         }
//     }
//     // Only Moore models can influence Mealy models.
//     allow_mealy_input = false;
//     // Iterate over activated Mealy models to calculate their output
//     for (auto m_iter : mealy) {
//         MealyAtomic<OutputType, TimeType>* model = m_iter;
//         assert(model->outputs->empty());

//         // Put it in the active set if it is not already there
//         if (!model->activated) {
//             activated.push_back(model);
//             model->activated = true;
//         }
//         // Compute output functions and route the events.
//         if (model
//                 ->imminent)  // These are the imminent Mealy models
//         {
//             if (!model->activated) {
//                 model->typeIsAtomic()->output_func(
//                     *(model->outputs));
//             } else {
//                 model->output_func(*(model->inputs),
//                                    *(model->outputs));
//             }
//         } else {
//             assert(model->activated);
//             // These are the Mealy models activated by input
//             model->output_func(sched.minPriority() - model->tL,
//                                *(model->inputs),
//                                *(model->outputs));
//         }
//     }
//     // Translate Mealy output to inputs for Moore models. The route method
//     // will throw an exception if an event is sent to a Mealy model.
//     for (auto m_iter : mealy) {
//         MealyAtomic<OutputType, TimeType>* model = m_iter;
//         // Route each event in y
//         for (typename list<OutputType>::iterator y_iter =
//                  model->outputs->begin();
//              y_iter != model->outputs->end(); y_iter++) {
//             route(model->getParent(), model, *y_iter);
//         }
//     }
//     mealy.clear();
//     io_up_to_date = true;
// }

// template <class OutputType, class TimeType>
// void Simulator<OutputType, TimeType>::computeNextOutput() {
//     computeNextOutput(bogus_input, sched.minPriority());
// }

// template <class OutputType, class TimeType>
// TimeType Simulator<OutputType, TimeType>::computeNextState(
//     list<Event<OutputType, TimeType>> & input, TimeType t) {
//     computeNextOutput(input, t);
//     assert(io_time == t && io_up_to_date);
//     return computeNextState();
// }

// template <class OutputType, class TimeType>
// TimeType Simulator<OutputType, TimeType>::computeNextState() {
//     if (!io_up_to_date) {
//         computeNextOutput();
//     }
//     io_up_to_date = false;
//     TimeType t = io_time,
//              tQ = io_time + adevs_epsilon<TimeType>();

//     // Track which models need and have been called
//     set<Devs<OutputType, TimeType>*> called;
//     list<Devs<OutputType, TimeType>*> pending_transitions;

//     /*
//      * Compute the states of atomic models.  Store Network models that
//      * need to have their model transition function evaluated in a
//      * special container that will be used when the structure changes are
//      * computed.
//      */
//     for (auto model : activated) {
//         // Internal event if no input
//         if (model->inputs->empty()) {
//             model->delta_int();
//         } else if (model->imminent) {
//             // Confluent event if model is imminent and has input
//             model->delta_conf(*(model->inputs));
//         } else {
//             // External event if model is not imminent and has input
//             model->delta_ext(t - model->tL, *(model->inputs));
//         }
//         // Notify listeners
//         this->notify_state_listeners(model, tQ);
//         // Check for a model transition

//         // Adjust position in the schedule
//         schedule(model, tQ);

//         bool propagate = model->model_transition();
//         auto parent = model->getParent();
//         if (propagate && parent != nullptr) {
//             pending_transitions.push_back(parent);


// template <class OutputType, class TimeType>
// void Simulator<OutputType, TimeType>::schedule(
//     Devs<OutputType, TimeType> * model, TimeType t) {
//     model->simulator = this;
//     Atomic<OutputType, TimeType>* a =
//         model->typeIsAtomic();
//     if (a != nullptr) {
//         a->tL = t;
//         TimeType dt = a->ta();
//         if (dt == adevs_inf<TimeType>()) {
//             sched.schedule(a, adevs_inf<TimeType>());
//         } else {
//             TimeType tNext = a->tL + dt;
//             if (tNext < a->tL) {
//                 exception err("Negative time advance",
//                               a);
//                 throw err;
//             }
//             sched.schedule(a, tNext);
//         }
//     } else {
//         set<Devs<OutputType, TimeType>*> components;
//         model->typeIsNetwork()->getComponents(
//             components);
//         for (auto iter : components) {
//             schedule(iter, t);
//         }
//     }
// }

// template <class OutputType, class TimeType>
// void Simulator<OutputType, TimeType>::inject_event(
//     Atomic<OutputType, TimeType> * model,
//     OutputType & value) {
//     if (io_time < model->tL) {
//         exception err(
//             "Attempt to apply input in the past",
//             model);
//         throw err;
//     }
//     // If this is a Mealy model, add it to the list of models that
//     // will need their input calculated
//     if (model->typeIsMealyAtomic()) {
//         if (allow_mealy_input) {
//             // Add it to the list of its not already there
//             if (!model->activated && !model->imminent) {
//                 mealy.push_back(
//                     model->typeIsMealyAtomic());
//             }
//         } else {
//             exception err(
//                 "Mealy model coupled to a Mealy model",
//                 model);
//             throw err;
//         }
//     }
//     // Add the output to the model's list of output to be processed
//     if (!model->activated) {
//         if (!model->imminent) {
//             activated.push_back(model);
//             model->activated = true;
//         }
//         //model->inputs->clear();
//     }
//     this->notify_input_listeners(model, value, io_time);
//     model->inputs->push_back(value);
// }

}  // namespace adevs

#endif
