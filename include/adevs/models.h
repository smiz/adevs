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

#ifndef _adevs_models_h_
#define _adevs_models_h_

#include <cstdlib>
#include <list>
#include <memory>
#include <set>
#include <atomic>
#include "adevs/exception.h"
#include "adevs/time.h"

using namespace std;


namespace adevs {

/// \cond DEV
/// doxygen will ignore these declarations while
/// producing documentation for the user
template <typename OutputType, typename TimeType>
class Simulator;
template <typename OutputType, typename TimeType>
class Schedule;
template <typename OutputType, typename TimeType>
class MealyAtomic;
/// \endcond

/**
 * @brief A point of connection between components in a model; the pin
 * is conceptually similar to a pin in an electric circuit model.
 * 
 * The style of model coupling used in adevs is based
 * on the concept of pins as they are usually encountered
 * in modeling tools for electric circuits. In that modeling
 * paradigm, pins are places where wires can be attached to
 * create electric connections between components. In electric
 * circuit models, these wires are bidirectional. In adevs, the
 * connections between pins and between models and pins
 * have a fixed direction in which events are transmitted.
 * 
 * For example, consider a model with components A and B.
 * There are pins p and q. A feedback loop between A and B
 * could look something like this:
 * \verbatim
 * A --> p
 * B --> q
 * q --> A
 * p --> B
 * \endverbatim
 * 
 * @see Graph
 * @see PinValue
 */
class pin_t {
  public:
    /// @brief Create a pin.
    ///
    /// Each pin is unique identified and its identifier is preserved
    /// across copies and assignments of the pin. 
    pin_t() : id(atom++) {}
    /// @brief  Copy constructor.
    /// @param src  The pin whose identifier is copied.
    pin_t(pin_t const &src) : id(src.id) {}
    /// @brief  Assignment operator.
    /// @param src  The pin whose identifier is assigned.
    pin_t const &operator=(pin_t const &src) {
        id = src.id;
        return *this;
    }
    /// @brief Do these pins have the same identifier?
    /// @param src  The pin to compare against.
    /// @return True if the pins have the same identifier, false otherwise.
    bool operator==(pin_t const &src) const { return (id == src.id); }
    /// @brief Do these pins have different identifiers?
    /// @param src  The pin to compare against.
    /// @return True if the pins have different identifiers, false otherwise.
    bool operator!=(pin_t const &src) const { return (id != src.id); }
    /// @brief Compare the identifiers of two pins.
    /// @param src  The pin to compare against.
    /// @return True if this pin has a smaller or the same identifier, false otherwise.
    bool operator<=(pin_t const &src) const { return (id <= src.id); }
    /// @brief Compare the identifiers of two pins.
    /// @param src  The pin to compare against.
    /// @return True if this pin has a smaller identifier, false otherwise.
    bool operator<(pin_t const &src) const { return (id < src.id); }
    /// @brief Compare the identifiers of two pins.
    /// @param src  The pin to compare against.
    /// @return True if this pin has a larger or the same identifier, false otherwise.
    bool operator>=(pin_t const &src) const { return (id >= src.id); }
    /// @brief Compare the identifiers of two pins.
    /// @param src  The pin to compare against.
    /// @return True if this pin has a larger identifier, false otherwise.
    bool operator>(pin_t const &src) const { return (id > src.id); }
  private:
    static std::atomic<int> atom;
    int id;
};

/**
 * @brief Create an event that appears on a pin.
 *
 * An Atomic creates PinValue objects in its output
 * function and it consumes PinValue objects in its state
 * transition functions. The PinValue objects can also be 
 * injected into and extracted from a running Simulator so
 * that the simulation can be used as part of a larger program.
 * 
 * @see pin_t
 * @see Atomic
 * @see Graph
 */
template <typename ValueType>
class PinValue {
  public:
    /// @brief Default constructor.
    PinValue() {}
    /// @brief Constructor that assigns a pin and value.
    /// @param pin  The pin on which the value appears.
    /// @param value  The value that appears on the pin.
    PinValue(pin_t pin, ValueType value) : pin(pin), value(value) {}
    /// @brief Copy constructor.
    /// @param src  The PinValue object to copy. The source objects
    ///             pin and value are copied using their copy constructors.
    PinValue(PinValue const &src) : pin(src.pin), value(src.value) {}
    /// @brief Assignment operator.
    /// @param src  The PinValue object to assign. The source object's
    ///             pin and value are assigned using their assignment operators.
    PinValue<ValueType> const &operator=(PinValue const &src) {
        pin = src.pin;
        value = src.value;
        return *this;
    }
    /// @brief The pin on which the value appears.
    pin_t pin;
    /// @brief The value that appears on the pin.
    ValueType value;
};


/** 
 * @brief An atomic model in the DEVS formalism.
 * 
 * An atomic model has a state that evolves over time. The basic building
 * block of any simulation program will be the behaviors of the Atomic components
 * that you create to define the active pieces of your model. The state
 * of an Atomic model changes in three ways:
 * - An internal state transition. This is a change in state that
 * the model undergoes all by itself, without any outside stimulation.
 * Internal state transitions are scheduled by the time advance function ta().
 * - An external state transition. This is a change in state that occurs
 * in response to an input, which is also called an external event.
 * - A confluent state transition. This is a change in state that occurs
 * when an input arrives at the same time as the model is scheduled to undergo
 * an internal state transition. Hence, the confluent state transition decides
 * what happens when the conditions for an internal and external state
 * transition are satisfied at the same time.
 *
 * The time advance function ta() informs the Simulator of when internal 
 * state transitions should occur. The time advance returns the amount of
 * time until the next internal state transition relative to the time at
 * which the previous state transition occurred.
 * 
 * For example, suppose the model changes from state A to state B at time t.
 * This change happens because the Simulator called the delta_int(), delta_ext(),
 * or delta_conf() method at time t. After calling one of these methods to
 * calculate the new state of the component, the Simulator will call the time 
 * advance method ta(). The return value of ta() is used by the Simulator to
 * schedule the next internal event for time t+ta().
 *
 * Now, if the simulation reaches time t + ta() without the model receiving any input.
 * Then at this time the Simulator calls delta_int() to calculate the models new state,
 * say state C. If instead the model receives an input x at time t + ta(), then the 
 * Simulator calculates the new state by calling the method delta_conf(x).
 * 
 * Suppose instead that the model receives an input x at some time t + e,, where e < ta().
 * In this case, the Simulator will call the method delta_ext(e, x) to calculate
 * the model's new state. Now the model is in a new state, say C, and the next
 * internal event will occur at time t + e + ta(). 
 * 
 * The output function output_func() is called by the Simulator to let the Atomic
 * model generate PinValue objects. The output function is called immediately
 * before the Simulator calls the delta_int() or delta_conf(). It is not called
 * prior to calling the delta_ext() method.
 * 
 * @see PinValue
 * @see Graph
 * @see Simulator
 */
template <typename OutputType, typename TimeType = double>
class Atomic {
  public:
    /// @brief The constructor should place the model into its initial state.
    Atomic()
        : tL(adevs_zero<TimeType>()),
          q_index(0) {}  // The Schedule requires this to be zero
    virtual ~Atomic() {}
    /**
     * @brief The internal transition function.
     * 
     * This is called by the Simulator when an amount of time equal to ta()
     * has passed since the last state change via the delta_int(), delta_ext(),
     * or delta_conf() method.
     */
    virtual void delta_int() = 0;
    /***
     * @brief The external transition function.
     * 
     * This is called by the Simulator when an input arrives before the next
     * scheduled to the delta_int() method.
     * 
     * @param e Time elapsed since the previous change of state.
     * @param xb A list of input for the model.
     */
    virtual void delta_ext(TimeType e,
                           std::list<PinValue<OutputType>> const &xb) = 0;
    /***
     * @brief The confluent transition function.
     * 
     * This is called by the Simulator when an input arrives at the same time
     * as the next scheduled internal event.
     * 
     * @param xb A list of input for the model.
     */
    virtual void delta_conf(std::list<PinValue<OutputType>> const &xb) = 0;
    /***
     * @brief The output function.
     * 
     * Output values should be added to the supplied list. Recall
     * that this method is called by the Simulator immediately before
     * the delta_int() or delta_conf() method is called.
     * @param yb Empty list to be filled with the model's output.
     */
    virtual void output_func(std::list<PinValue<OutputType>> &yb) = 0;
    /***
     * @brief The time advance function.
     * 
     * This method is called by the Simulator immediately after any call to
     * delta_int(), delta_ext(), or delta_conf(). The return value is used
     * to schedule the next internal event. Return adevs_inf<TimeType>() for infinity.
     * 
     * @return The time to the next internal event.
     */
    virtual TimeType ta() = 0;

  private:
    friend class Simulator<OutputType, TimeType>;
    friend class Schedule<OutputType, TimeType>;

    // Time of last event
    TimeType tL, tN;
    // Index in the priority queue
    unsigned int q_index;

    std::list<PinValue<OutputType>> inputs;
    std::list<PinValue<OutputType>> outputs;

    virtual MealyAtomic<OutputType, TimeType>* isMealyAtomic() { return nullptr; }

};


// Clang complains about the output_func declaration.
// Because what we wrote is what we intended, the
// warning is disable just for this class definition.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif

/**
 * @brief A Mealy type atomic model where its output
 * may depend on its input.
 * 
 * Mealy models cannot appear in loops that contain
 * other Mealy models. The simulator will throw an
 * exception and abort if you attempt to do so.
 */
template <typename OutputType, typename TimeType = double>
class MealyAtomic : public Atomic<OutputType, TimeType> {
  public:
    /// @brief Default constructor.
    MealyAtomic<OutputType, TimeType>() : Atomic<OutputType, TimeType>() {}
    /**
     * @brief Produce output at an external transition.
     * 
     * This method is called when the model receives input before its
     * next internal event. The elapsed time and input will be the same as that
     * passed to the delta_ext() method.
     * @param e The elapsed time since the last state change.
     * @param xb The input values that arrived at the model.
     * @param yb The output values produced by the model.
     */
    virtual void external_output_func(TimeType e, std::list<PinValue<OutputType>> const &xb,
                             std::list<PinValue<OutputType>> &yb) = 0;
    /**
     * #@brief Produce output at a confluent transition.
     * 
     * This method is called when the model receives input at the same time
     * as its next internal event. The input will be the same that is passed
     * to the delta_conf() method.
     *
     * @param xb The input values that arrived at the model.
     * @param yb The output values produced by the model.
     */
    virtual void confluent_output_func(std::list<PinValue<OutputType>> const &xb,
                             std::list<PinValue<OutputType>> &yb) = 0;
    /// @brief Destructor
    virtual ~MealyAtomic() {}

  private:
    friend class Simulator<OutputType, TimeType>;

    MealyAtomic<OutputType, TimeType>* isMealyAtomic() { return this; }

};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

}  // namespace adevs

#endif
