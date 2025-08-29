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

#include <list>
#include <memory>
#include <set>
#include <atomic>
#include <any>
#include "adevs/exception.h"
#include "adevs/time.h"

namespace adevs {

/// \cond DEV
/// doxygen will ignore these declarations while
/// producing documentation for the user
template <typename ValueType, typename TimeType>
class Simulator;
template <typename ValueType, typename TimeType>
class Schedule;
template <typename ValueType, typename TimeType>
class MealyAtomic;
template <typename ValueType, typename TimeType>
class Graph;
/// \endcond

/**
 * @brief A point of connection between components in a model.
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
 * A component places output on a pin by creating a PinValue object
 * and adding it to the list of output values in Atomic::output_func().
 * A component receives input on a pin where Graph::connect() or Coupled::create_coupling()
 * methods are called such that a path exists from the pin used in the
 * Atomic::output_func() method to the Atomic model that receives the input.
 *
 * @see Atomic
 * @see Coupled  
 * @see Graph
 * @see PinValue
 */
class pin_t {
  public:
    /// @brief Create a pin.
    ///
    /// Each pin is unique and its identity is preserved
    /// across copies and assignments of the pin. 
    pin_t() : id(atom++) {}
    /// @brief  Copy constructor.
    /// @param src  The pin whose identity is copied.
    pin_t(pin_t const &src) : id(src.id) {}
    /// @brief  Assignment operator.
    /// @param src  The pin whose identity is assigned.
    pin_t const &operator=(pin_t const &src) {
        id = src.id;
        return *this;
    }
    /// @brief Do these pins have the same identity?
    /// @param src  The pin to compare against.
    /// @return True if the pins have the same identity, false otherwise.
    bool operator==(pin_t const &src) const { return (id == src.id); }
    /// @brief Do these pins have different identities?
    /// @param src  The pin to compare against.
    /// @return True if the pins have different identities, false otherwise.
    bool operator!=(pin_t const &src) const { return (id != src.id); }
    /// @brief Compare the identities of two pins.
    /// @param src  The pin to compare against.
    /// @return True if this pin has a smaller or the same identity, false otherwise.
    bool operator<=(pin_t const &src) const { return (id <= src.id); }
    /// @brief Compare the identities of two pins.
    /// @param src  The pin to compare against.
    /// @return True if this pin has a smaller identity, false otherwise.
    bool operator<(pin_t const &src) const { return (id < src.id); }
    /// @brief Compare the identities of two pins.
    /// @param src  The pin to compare against.
    /// @return True if this pin has a larger or the same identity, false otherwise.
    bool operator>=(pin_t const &src) const { return (id >= src.id); }
    /// @brief Compare the identities of two pins.
    /// @param src  The pin to compare against.
    /// @return True if this pin has a larger identity, false otherwise.
    bool operator>(pin_t const &src) const { return (id > src.id); }
  private:
    static std::atomic<int> atom;
    int id;
};

/**
 * @brief An event that appears on a pin.
 *
 * An Atomic creates PinValue objects in its Atomic::output_func()
 * method and it consumes PinValue objects in its state
 * transition functions: Atomic::delta_int(), Atomic::delta_ext(), and
 * Atomic::delta_conf(). The PinValue objects can also be 
 * injected into and extracted from a running Simulator so
 * that the simulation can be used as part of a larger program.
 * 
 * @see pin_t
 * @see Atomic
 * @see Graph
 * @see Coupled
 * @see Simulator
 */
template <typename ValueType = std::any>
class PinValue {
  public:
    /// @brief Default constructor.
    PinValue() {}
    /// @brief Constructor that assigns a pin and value.
    /// @param pin  The pin on which the value appears.
    /// @param value  The value that appears on the pin.
    PinValue(pin_t pin, ValueType value) : pin(pin), value(value) {}
    /// @brief Copy constructor.
    ///
    /// The source object's pin and value are copied using their
    /// copy constructors.
    ///
    /// @param src  The PinValue object to copy.
    PinValue(PinValue const &src) : pin(src.pin), value(src.value) {}
    /// @brief Assignment operator.
    ///
    /// The source object's pin and value are assigned using their
    /// assignment operators.
    ///
    /// @param src  The PinValue object to assign. 
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
 * An Atomic model has a state that evolves over time. The basic building
 * block of any simulation program will be the behaviors of the Atomic components
 * that you create to define the active pieces of your model. The state
 * of an Atomic model changes in three ways:
 * - An internal state transition using Atomic::delta_int(). This is a change of state that
 * the model undergoes all by itself, without any outside stimulation.
 * - An external state transition using Atomic::delta_ext(). This is a change of state that occurs
 * in response to an input, which is also called an external event.
 * - A confluent state transition using Atomic::delta_conf(). This is a change of state that occurs
 * when an input arrives at the same time that the model is scheduled to undergo
 * an internal state transition. Hence, the confluent state transition decides
 * what happens when the conditions for an internal and external state
 * transition are satisfied at the same time.
 *
 * The time advance function Atomic::ta() informs the Simulator of when internal 
 * state transitions should occur. The time advance returns the amount of
 * time until the next internal state transition relative to the time at
 * which the previous state transition occurred.
 * 
 * For example, suppose the model changes from state A to state B at time t.
 * This change happens because the Simulator called Atomic::delta_int(), Atomic::delta_ext(),
 * or Atomic::delta_conf() at time t. After calling one of these methods to
 * calculate the new state of the component, the Simulator calls the time 
 * advance method Atomic::ta(). The return value is used by the Simulator to
 * schedule the next internal event for time t+Atomic::ta().
 *
 * Now, suppose the simulation reaches time t + Atomic::ta() without the model receiving any input.
 * Then at this time the Simulator calls Atomic::delta_int() to calculate the model's new state,
 * say state C. If instead the model receives an input x at time t + Atomic::ta(), then the 
 * Simulator calculates the new state by calling Atomic::delta_conf(x).
 * 
 * Suppose instead that the model receives an input x at some time t + e where e < Atomic::ta().
 * In this case, the Simulator will call the method Atomic::delta_ext() with arguments e and x to calculate
 * the model's new state. Now the model is in a new state, say C. The Simulator
 * calls the Atomic::ta() method to get the time advance and the next internal event is
 * scheduled for time t + e + Atomic::ta(). 
 * 
 * The output function Atomic::output_func() is called by the Simulator to let the Atomic
 * model generate PinValue objects. The output function is called immediately
 * before the Simulator calls the Atomic::delta_int() or Atomic::delta_conf(). It is not called
 * prior to calling the Atomic::delta_ext() method.
 * 
 * @see PinValue
 * @see Graph
 * @see Coupled
 * @see Simulator
 */
template <typename ValueType = std::any, typename TimeType = double>
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
     * scheduled call to the delta_int() method.
     * 
     * @param e Time elapsed since the previous change of state.
     * @param xb A list of input for the model.
     */
    virtual void delta_ext(TimeType e,
                           std::list<PinValue<ValueType>> const &xb) = 0;
    /***
     * @brief The confluent transition function.
     * 
     * This is called by the Simulator when an input arrives at the same time
     * as the next scheduled internal event.
     * 
     * @param xb A list of input for the model.
     */
    virtual void delta_conf(std::list<PinValue<ValueType>> const &xb) = 0;
    /***
     * @brief The output function.
     * 
     * Output values should be added to the supplied list. Recall
     * that this method is called by the Simulator immediately before
     * the delta_int() or delta_conf() method is called.
     * 
     * @param yb Empty list to be filled with the model's output.
     */
    virtual void output_func(std::list<PinValue<ValueType>> &yb) = 0;
    /***
     * @brief The time advance function.
     * 
     * This method is called by the Simulator immediately after any call to
     * delta_int(), delta_ext(), or delta_conf(). The return value is used
     * to schedule the next internal event. Return adevs_inf<TimeType>()
     * for infinity; that is, to have no scheduled internal event.
     * 
     * @return The time to the next internal event.
     */
    virtual TimeType ta() = 0;

  private:
    friend class Simulator<ValueType, TimeType>;
    friend class Schedule<ValueType, TimeType>;

    // Time of last event
    TimeType tL, tN;
    // Index in the priority queue
    unsigned int q_index;

    std::list<PinValue<ValueType>> inputs;
    std::list<PinValue<ValueType>> outputs;

    virtual MealyAtomic<ValueType, TimeType>* isMealyAtomic() { return nullptr; }

};


// Clang complains about the output_func declaration.
// Because what we wrote is what we intended, the
// warning is disable just for this class definition.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif

/**
 * @brief A Mealy type atomic model whose output
 * may depend on its input.
 * 
 * Mealy models cannot appear in loops that contain
 * other Mealy models. The simulator will throw an
 * adevs::exception and abort if you attempt to do so.
 */
template <typename ValueType = std::any, typename TimeType = double>
class MealyAtomic : public Atomic<ValueType, TimeType> {
  public:
    /// @brief Default constructor.
    MealyAtomic() : Atomic<ValueType, TimeType>() {}
    /**
     * @brief Produce output at an external transition.
     * 
     * This method is called when the model receives input before its
     * next internal event. The elapsed time and input are the same as
     * are passed to the delta_ext() method. This is called before the call to
     * delta_ext().
     * 
     * @param e The elapsed time since the last state change.
     * @param xb The input values that arrived at the model.
     * @param yb The output values produced by the model.
     */
    virtual void external_output_func(TimeType e, std::list<PinValue<ValueType>> const &xb,
                             std::list<PinValue<ValueType>> &yb) = 0;
    /**
     * @brief Produce output at a confluent transition.
     * 
     * This method is called when the model receives input at the same time
     * as its next internal event. The input will be the same that is passed
     * to the delta_conf() method. This method is called before delta_conf().
     *
     * @param xb The input values that arrived at the model.
     * @param yb The output values produced by the model.
     */
    virtual void confluent_output_func(std::list<PinValue<ValueType>> const &xb,
                             std::list<PinValue<ValueType>> &yb) = 0;
    /// @brief Destructor
    virtual ~MealyAtomic() {}

  private:
    friend class Simulator<ValueType, TimeType>;

    MealyAtomic<ValueType, TimeType>* isMealyAtomic() { return this; }

};

#ifdef __clang__
#pragma clang diagnostic pop
#endif

/**
 * @brief A coupled model in the DEVS formalism.
 * 
 * A Coupled model is composed of Atomic models and other Coupled models.
 * It is used to construct complex components by interconnecting simpler
 * piece parts. The components and connections of the Coupled model are imposed
 * on a Graph when the Coupled model is added to that Graph. Later changes to
 * the Coupled model, such as adding or removing components or couplings, are
 * reflected in the Graph to which the Coupled model belongs.
 *
 * The Coupled model does not require strict hierarchy. An Atomic or Coupled
 * model can be a member of more than one Coupled model. However, edges
 * between pins, between pins and Atomic models, and the Atomic models themselves
 * will only appear once in the underlying Graph. See the Graph documentation
 * for how multiple additions of the same Atomic model or edge is handled. A
 * Graph is automatically assigned to the coupled model when it is handed to 
 * the constructor of a Simulator.
 * 
 * @see Graph
 * @see Atomic
 */
template <typename ValueType = std::any, typename TimeType = double>
class Coupled {
  public:
    /**
     * @brief Constructor creates a model without components or couplings.
     */
    Coupled() : g(nullptr) {}
    /**
     * @brief Destructor
     *
     * The destructor does not remove the coupled models components
     * and couplings from the underlying Graph. If you want to do this
     * it must be done explicitly before destroying the Coupled model.
     */ 
    virtual ~Coupled(){}

    /**
     * @ brief Add an Atomic model to the Coupled model.
     *
     * The Atomic model becomes part of the Coupled model. We do not
     * enforce strict hierarchy and an Atomic model may belong to more
     * than one Coupled model. 
     *  
     * @param model The Atomic model to add.
     */
    void add_atomic(std::shared_ptr<Atomic<ValueType, TimeType>> model);
    /**
     * @brief Remove an Atomic model.
     * 
     * This removes the Atomic model and connections to it from this
     * model. Only the connections that are part of this Coupled model
     * are removed from the underlying Graph. The Atomic is removed
     * from the underlying Graph only if this is the only instance of
     * the model in the Graph.
     *
     * @see Graph
     *  
     * @param model The Atomic model to remove.
     */
    void remove_atomic(std::shared_ptr<Atomic<ValueType, TimeType>> model);
    /**
     * @brief Add a Coupled model to this model.
     * 
     * As with add_atomic() but for Coupled models.
     * 
     * @param model The Coupled model to add.
     */
    void add_coupled_model(std::shared_ptr<Coupled<ValueType, TimeType>> model);
    /**
     * @brief Remove a Coupled model from this model.
     * 
     * This removes from the underlying Graph all of the components and couplings
     * of the Coupled model that is being removed. This method recursively removes
     * the Coupled model and components of all Coupled models that are in the tree
     * of models that have this model as its root. If any component appears more
     * than once in the underlying Graph, then it is not removed from the Graph. However,
     * the connections associated with the removed hierarchy are removed from the Graph.
     * 
     * @param model The Coupled model to remove.
     */
    void remove_coupled_model(std::shared_ptr<Coupled<ValueType, TimeType>> model);
    /**
     * @brief Create a coupling in the model.
     * 
     * This method connects a pin to an Atomic model that is part of this 
     * Coupled model.
     * 
     * @param pin The pin.
     * @param model The Atomic model that receives input on the destination pin.
     */
    void create_coupling(pin_t pin, std::shared_ptr<Atomic<ValueType, TimeType>> model);
    /** 
     * @brief Add a coupling between pins in the Coupled model.
     * 
     * @param src The source pin.
     * @param dst The destination pin.
     */
    void create_coupling(pin_t src, pin_t dst);
    /**
     * @brief Remove a coupling in the model.
     * 
     * @param pin The pin.
     * @param model The Atomic model that receives input on the destination pin.
     */
    void remove_coupling(pin_t pin, std::shared_ptr<Atomic<ValueType, TimeType>> model);
    /** 
     * @brief Remove a coupling between pins in the Coupled model.
     * 
     * @param src The source pin.
     * @param dst The destination pin.
     */
    void remove_coupling(pin_t src, pin_t dst);

  private:

    friend class Simulator<ValueType, TimeType>;

    Graph<ValueType, TimeType>* g;
    std::set<std::shared_ptr<Atomic<ValueType, TimeType>>> atomic_components;
    std::set<std::shared_ptr<Coupled<ValueType, TimeType>>> coupled_components;
    /// Pins that provide input to the Atomic models 
    std::set<std::pair<pin_t,std::shared_ptr<Atomic<ValueType, TimeType>>>> pin_to_atomic;
    /// Pin to pin connections
    std::set<std::pair<pin_t,pin_t>> pin_to_pin;

    void assign_to_graph(Graph<ValueType, TimeType>* graph);
    void remove_from_graph();
};

template <typename ValueType, typename TimeType>
void Coupled<ValueType, TimeType>::create_coupling(pin_t pin, std::shared_ptr<Atomic<ValueType, TimeType>> model) {
    pin_to_atomic.insert(std::make_pair(pin, model));
    if (g != nullptr) {
        g->connect(pin, model);
    }
}

template <typename ValueType, typename TimeType>
void Coupled<ValueType, TimeType>::remove_coupling(pin_t pin, std::shared_ptr<Atomic<ValueType, TimeType>> model) {
    pin_to_atomic.erase(std::make_pair(pin, model));
    if (g != nullptr) {
        g->disconnect(pin, model);
    }
}

template <typename ValueType, typename TimeType>
void Coupled<ValueType, TimeType>::create_coupling(pin_t src, pin_t dst) {
    pin_to_pin.insert(std::make_pair(src, dst));
    if (g != nullptr) {
        g->connect(src, dst);
    }
}

template <typename ValueType, typename TimeType>
void Coupled<ValueType, TimeType>::remove_coupling(pin_t src, pin_t dst) {
    pin_to_pin.erase(std::make_pair(src, dst));
    if (g != nullptr) {
        g->disconnect(src, dst);
    }
}

template <typename ValueType, typename TimeType>
void Coupled<ValueType, TimeType>::assign_to_graph(Graph<ValueType, TimeType>* graph) {
    g = graph;
    for (auto atomic : atomic_components) {
        g->add_atomic(atomic);
    }
    for (auto coupling: pin_to_atomic) {
        g->connect(coupling.first, coupling.second);
    }
    for (auto coupling: pin_to_pin) {
        g->connect(coupling.first, coupling.second);
    }
    for (auto coupled : coupled_components) {
        coupled->assign_to_graph(g);
    }
}

template <typename ValueType, typename TimeType>
void Coupled<ValueType, TimeType>::remove_from_graph() {
    for (auto atomic : atomic_components) {
        g->remove_atomic(atomic);
    }
    for (auto coupling: pin_to_atomic) {
        g->disconnect(coupling.first, coupling.second);
    }
    for (auto coupling: pin_to_pin) {
        g->connect(coupling.first, coupling.second);
    }
    for (auto coupled : coupled_components) {
        coupled->remove_from_graph();
    }
}

template <typename ValueType, typename TimeType>
void Coupled<ValueType, TimeType>::add_atomic(std::shared_ptr<Atomic<ValueType, TimeType>> model) {
    atomic_components.insert(model);
    if (g != nullptr) {
        g->add_atomic(model);
    }
}

template <typename ValueType, typename TimeType>
void Coupled<ValueType, TimeType>::remove_atomic(std::shared_ptr<Atomic<ValueType, TimeType>> model) {
    atomic_components.erase(model);
    if (g != nullptr) {
        g->remove_atomic(model);
    }
}

template <typename ValueType, typename TimeType>
void Coupled<ValueType, TimeType>::add_coupled_model(std::shared_ptr<Coupled<ValueType, TimeType>> model) {
    coupled_components.insert(model);
    if (g != nullptr) {
        model->assign_to_graph(g);
    }
}

template <typename ValueType, typename TimeType>
void Coupled<ValueType, TimeType>::remove_coupled_model(std::shared_ptr<Coupled<ValueType, TimeType>> model) {
    coupled_components.erase(model);
    if (g != nullptr) {
        model->remove_from_graph();
    }
}

}  // namespace adevs

#endif
