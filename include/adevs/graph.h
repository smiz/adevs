
/*
 * Copyright (c) 2025, James Nutaro
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
#ifndef _adevs_graph_h_
#define _adevs_graph_h_
#include <map>
#include <list>
#include <set>
#include <memory>
#include "adevs/models.h"

namespace adevs {

/**
 * @brief A directed graph for connecting atomic models to pins
 * and pins and pins.
 * 
 * The graph is used to connect Atomic models to each other. If there
 * is a path through the graph from an Atomic model A to an Atomic model B,
 * then events created by A on the first pin in that path will be delivered
 * as input to B via the last pin in that path. If the Simulator injects
 * an event on a pin from which there is a path to an Atomic model, then
 * that event will be delivered to the Atomic model as input via the last
 * pin on the path.
 * 
 * The graph operates in one of two modes: provisional or non-provisional. In
 * the non-provisional mode, changes that are made to the graph take place
 * immediately. This mode is used primarily when the graph is being constructed
 * prior to running a simulation. In the provisional mode, changes to the Graph
 * are queued and then applied at the end of the current simulation cycle,
 * immediately after the computations of state changes are finished. The
 * provisional mode is managed by the Simulator.
 * 
 * @see Atomic
 * @see PinValue
 * @see Simulator
*/
template <typename X, typename T = double>
class Graph {
    public:
        /**
         * @brief  Construct an empty graph.
         * 
         * The new Graph is in the non-provisional mode.
         */
        Graph():provisional(false){}
        /// @brief Destroy the graph but leave its atomic components intact.
        virtual ~Graph(){}
        /**
         * @brief  Remove a pin.
         * 
         * All edges connected to this pin are removed from the graph.
         * @param p The pin to remove.
         */
        void remove_pin(pin_t p);
        /// @brief Add an atomic model to the graph.
        /// @param model The model to add.
        void add_atomic(std::shared_ptr<Atomic<X,T>> model);
        /**
         *  @brief Remove an atomic model from the graph.
         * 
         *  The model and all edges from a pin to the model are removed from the graph.
         *  @param model The model to remove.
         */
        void remove_atomic(std::shared_ptr<Atomic<X, T>> model);
        /**
         * @brief Connect two pins in the graph.
         * 
         * Create a link from the source pin the destination pin.
         * Any event placed on the source pin will be transmitted
         * to the destination pin.
         * @param src The source pin.
         * @param dst The destination pin.
         */
        void connect(pin_t src, pin_t dst);
        /**
         * @brief  Remove a connection between two pins.
         * 
         * This removes the link from the source pin to the destination pin.
         * @param src The source pin.
         * @param dst The destination pin.
         */
        void disconnect(pin_t src, pin_t dst);
        /**
         * @brief  Connect a pin to an atomic model.
         * 
         * Events appearing on the pin will be delivered to the atomic model as input.
         * The Atomic model must already have been added to the graph.
         * @param pin The pin to connect.
         * @param model The model to receive the input.
         */
        void connect(pin_t pin, std::shared_ptr<Atomic<X, T>> model);
        /**
         * @brief  Remove a connection from a pin and to an Atomic model.
         * 
         * This removes the link from the pin to the Atomic model.
         * @param pin The pin to disconnect.
         * @param model The model to disconnect.
         */
        void disconnect(pin_t pin, std::shared_ptr<Atomic<X, T>> model);
        /**
         * @brief  Get the Atomic models that are connected to a pin.
         * 
         * This fills the supplied list with Atomic model and pin pairs. The pairs
         * in the list are the pin to Atomic connections added with connect(pin_t, Atomic<X,T>).
         * The list contains just those pin and Atomic pairs for which their is a path
         * from the supplied pin.
         * @param pin The pin to query.
         * @param models A list to be filled with models and pins that are connected to the supplied pin.
         */
        void route(pin_t pin, std::list<std::pair<pin_t,std::shared_ptr<Atomic<X, T>>>>& models) const;
        /**
         *  @brief  Get the set of Atomic models that are part of the graph.
         * 
         * This returns a set of all Atomic models that have been added to the graph.
         * @return The set of all Atomic models.
         */
        const std::set<std::shared_ptr<Atomic<X,T>>>& get_atomics() const { return models; }
        /**
         * @brief Enable or disable the provisional model.
         * 
         * This is used by the Simulator to indicate that the graph is in a provisional state.
         * The Simulator will always put the Graph into a provisional state when it is calculating
         * state transition functions and will set the graph to non-provisional when those 
         * calculations are complete. In general, the modeler is not expected to use this method.
         * 
         * @param p If true, the graph is in the provisional mode.
         */ 
        /// The graph must always be in a provisional state when a simulation is running.
        void set_provisional(bool p) { provisional = p; }

        /// Types of queued operations.
        enum pending_op {
            /// @brief  A pending call to add_atomic().
            ADD_ATOMIC,
            /// @brief  A pending call to remove_atomic().
            REMOVE_ATOMIC,
            /// @brief  A pending call to remove_pin().
            REMOVE_PIN,
            /// @brief  A pending call to connect(pin_t, pin_t).
            CONNECT_PIN_TO_PIN,
            /// @brief  A pending call to disconnect(pin_t, pin_t).
            DISCONNECT_PIN_FROM_PIN,
            /// @brief  A pending call to connect(pin_t, shared_ptr<Atomic<X,T>>).
            CONNECT_PIN_TO_ATOMIC,
            /// @brief  A pending call to disconnect(pin_t, shared_ptr<Atomic<X,T>>).
            DISCONNECT_PIN_FROM_ATOMIC,
        };
        /**
         * @brief  This describes a pending operation that has been queued while the graph is in the provisional mode.
         */
        struct graph_op
        {
            /// The type of operation that has been queued.
            pending_op op;
            /**
             * @brief The pin that is involved in the operation.
             * 
             * The zeroth entry corresponds to the first pin in the operation.
             * The first entry corresponds to the second pin in the operation, if
             * applicable.
             */
            pin_t pin[2];
            /// @brief The Atomic model that is involved in the operation, if any.
            std::shared_ptr<Atomic<X,T>> model;
        };
        /**
         * @brief Get the set of operations that have been queued while in the provisional mode.
         * 
         * To commit the items in this list, set_provisional(false) must be called before
         * calling the method that processes the pending operations. The queued operations
         * must be removed from the list by the caller after they have been processed.
         * 
         * @return The list of pending operations.
         */
        std::list<graph_op>& get_pending() { return pending; }

    private:
        std::map<pin_t,std::list<std::shared_ptr<Atomic<X,T>>>> pin_to_atomic;
        std::map<pin_t,std::list<pin_t> > pin_to_pin;
        std::set<std::shared_ptr<Atomic<X,T>>> models;

        void queue_remove_pin(pin_t p);
        void queue_add_atomic(std::shared_ptr<Atomic<X,T>> model);
        void queue_remove_atomic(std::shared_ptr<Atomic<X, T>> model);
        void queue_connect(pin_t src, pin_t dst);
        void queue_disconnect(pin_t src, pin_t dst);
        void queue_connect(pin_t pin, std::shared_ptr<Atomic<X, T>> model);
        void queue_disconnect(pin_t pin, std::shared_ptr<Atomic<X, T>> model);

        // Pending changes are provisional? This is used by the simulator
        // to indicate that the graph is in a provisional state during execution.
        // When true, changes are queued but not applied to the graph.
        bool provisional;
        std::list<graph_op> pending;
    };

template <typename X, typename T>
void Graph<X, T>::remove_pin(pin_t pin) {
    if (provisional) {
        queue_remove_pin(pin);
        return;
    }
    pin_to_atomic.erase(pin);
    pin_to_pin.erase(pin);
    for (auto i = pin_to_pin.begin(); i != pin_to_pin.end(); i++) {
        i->second.remove(pin);
    }
}

template <typename X, typename T>
void Graph<X, T>::add_atomic(std::shared_ptr<Atomic<X,T>> model) {
    if (provisional) {
        queue_add_atomic(model);
        return;
    }
    models.insert(model);
}

template <typename X, typename T>
void Graph<X, T>::remove_atomic(std::shared_ptr<Atomic<X,T>> model) {
    if (provisional) {
        queue_remove_atomic(model);
        return;
    }
    for (auto i = pin_to_atomic.begin(); i != pin_to_atomic.end(); i++) {
        i->second.remove(model);
    }
    models.erase(model);
}

template <typename X, typename T>
void Graph<X, T>::connect(pin_t src, pin_t dst) {
    if (provisional) {
        queue_connect(src, dst);
        return;
    }
    pin_to_pin[src].push_back(dst);
}

template <typename X, typename T>
void Graph<X, T>::disconnect(pin_t src, pin_t dst) {
    if (provisional) {
        queue_disconnect(src, dst);
        return;
    }
    auto i = pin_to_pin.find(src);
    if (i != pin_to_pin.end()) {
        i->second.remove(dst);
    }
}

template <typename X, typename T>
void Graph<X, T>::connect(pin_t pin, std::shared_ptr<Atomic<X,T>> model) {
    if (provisional) {
        queue_connect(pin, model);
        return;
    }
    models.insert(model);
    pin_to_atomic[pin].push_back(model);
}

template <typename X, typename T>
void Graph<X, T>::disconnect(pin_t pin, std::shared_ptr<Atomic<X,T>> model) {
    if (provisional) {
        queue_disconnect(pin, model);
        return;
    }
    auto i = pin_to_atomic.find(pin);
    if (i != pin_to_atomic.end()) {
        i->second.remove(model);
    }
}

template <typename X, typename T>
void Graph<X,T>::route(pin_t pin, std::list<std::pair<pin_t,std::shared_ptr<Atomic<X, T>>>>& models) const {
    auto i = pin_to_atomic.find(pin);
    if (i != pin_to_atomic.end()) {
        for (auto j = i->second.begin(); j != i->second.end(); j++) {
            models.push_back(std::pair<pin_t,std::shared_ptr<Atomic<X, T>>>(pin,*j));
        }
    }
    auto r = pin_to_pin.find(pin);
    if (r != pin_to_pin.end()) {
        for (auto s = r->second.begin(); s != r->second.end(); s++) {
            route(*s, models);
        }
    }
}

template <typename X, typename T>
void Graph<X, T>::queue_add_atomic(std::shared_ptr<Atomic<X,T>> model) {
    graph_op op;
    op.op = ADD_ATOMIC;
    op.model = model;
    pending.push_back(op);
}

template <typename X, typename T>
void Graph<X, T>::queue_remove_pin(pin_t pin) {
    graph_op op;
    op.op = REMOVE_PIN;
    op.pin[0] = pin;
    pending.push_back(op);
}

template <typename X, typename T>
void Graph<X, T>::queue_remove_atomic(std::shared_ptr<Atomic<X,T>> model) {
    graph_op op;
    op.op = REMOVE_ATOMIC;
    op.model = model;
    pending.push_back(op);
}

template <typename X, typename T>
void Graph<X, T>::queue_connect(pin_t src, pin_t dst) {
    graph_op op;
    op.op = CONNECT_PIN_TO_PIN;
    op.pin[0] = src;
    op.pin[1] = dst;
    pending.push_back(op);
}

template <typename X, typename T>
void Graph<X, T>::queue_disconnect(pin_t src, pin_t dst) {
    graph_op op;
    op.op = DISCONNECT_PIN_FROM_PIN;
    op.pin[0] = src;
    op.pin[1] = dst;
    pending.push_back(op);
}

template <typename X, typename T>
void Graph<X, T>::queue_connect(pin_t pin, std::shared_ptr<Atomic<X,T>> model) {
    graph_op op;
    op.op = CONNECT_PIN_TO_ATOMIC;
    op.pin[0] = pin;
    op.model = model;
    pending.push_back(op);
}

template <typename X, typename T>
void Graph<X, T>::queue_disconnect(pin_t pin, std::shared_ptr<Atomic<X,T>> model) {
    graph_op op;
    op.op = DISCONNECT_PIN_FROM_ATOMIC;
    op.pin[0] = pin;
    op.model = model;
    pending.push_back(op);
}

}

#endif
