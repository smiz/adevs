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

/*
 * Declare network and atomic model so types can be used as the type of
 * parent in the basic Devs model and for type ID functions.
 */
template <typename OutputType, typename TimeType>
class Simulator;
template <typename OutputType, typename TimeType>
class Schedule;

class pin_t {
  public:
    pin_t() : id(atom++) {}
    pin_t(pin_t const &src) : id(src.id) {}
    pin_t const &operator=(pin_t const &src) {
        id = src.id;
        return *this;
    }
    bool operator==(pin_t const &src) const { return (id == src.id); }
    bool operator!=(pin_t const &src) const { return (id != src.id); }
    bool operator<=(pin_t const &src) const { return (id <= src.id); }
    bool operator<(pin_t const &src) const { return (id < src.id); }
    bool operator>=(pin_t const &src) const { return (id >= src.id); }
    bool operator>(pin_t const &src) const { return (id > src.id); }
  private:
    static std::atomic<int> atom;
    int id;
};

/**
 * The PinValue class is used for input and output in a simulation.
 * A pin can be attached to a model or to another pin. Events are
 * transmitted from pin to pin, from model to pin, and from pin to
 * model.
 */
template <typename ValueType>
class PinValue {
  public:
    /// @brief Default constructor.
    PinValue() {}
    /// @brief Constructor that assigns a pin and value.
    PinValue(pin_t pin, ValueType value) : pin(pin), value(value) {}
    /// @brief Copy constructor.
    PinValue(PinValue const &src) : pin(src.pin), value(src.value) {}
    /// @brief Assignment operator.
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


/*
 * Base type for all atomic DEVS models.
 */
template <typename OutputType, typename TimeType = double>
class Atomic {
  public:
    /// The constructor should place the model into its initial state.
    Atomic()
        : tL(adevs_zero<TimeType>()),
          q_index(0) {}  // The Schedule requires this to be zero
    virtual ~Atomic() {}
    /// Internal transition function.
    virtual void delta_int() = 0;
    /***
     * External transition function.
     * @param e Time elapsed since the previous change of state.
     * @param xb Input for the model.
     */
    virtual void delta_ext(TimeType e,
                           std::list<PinValue<OutputType>> const &xb) = 0;
    /***
     * Confluent transition function.
     * @param xb Input for the model.
     */
    virtual void delta_conf(std::list<PinValue<OutputType>> const &xb) = 0;
    /***
     * Output function.  Output values should be added to the list yb.
     * @param yb Empty list to be filled with the model's output.
     */
    virtual void output_func(std::list<PinValue<OutputType>> &yb) = 0;
    /***
     * Time advance function. Use adevs_inf<TimeType>() for infinity.
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
};

/*
 * This is a Mealy type atomic model where its output
 * may depend on its input. Mealy machines cannot be
 * connected to other Mealy machines. An exception
 * will be generated by the Simulator if you attempt
 * to do so.
 */


// Clang complains about the output_func declaration.
// Because what we wrote is what we intended, the
// warning is disable just for this class definition.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif

template <typename OutputType, typename TimeType = double>
class MealyAtomic : public Atomic<OutputType, TimeType> {
  public:
    MealyAtomic<OutputType, TimeType>() : Atomic<OutputType, TimeType>() {}
    MealyAtomic<OutputType, TimeType>* typeIsMealyAtomic() { return this; }
    /*
     * Produce output at e < ta(q) in response to xb.
     * This is output preceding an external event.
     */
    virtual void output_func(TimeType e, list<PinValue<OutputType>> const &xb,
                             list<PinValue<OutputType>> &yb) = 0;
    /*
     * Produce output at e = ta(q) in response to xb.
     * This is output preceding a confluent event.
     */
    virtual void output_func(list<OutputType> const &xb,
                             list<OutputType> &yb) = 0;

  private:
    friend class Simulator<OutputType, TimeType>;
};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

}  // namespace adevs

#endif
