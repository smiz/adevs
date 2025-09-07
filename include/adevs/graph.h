
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
 * The graph holds exactly one instance of each Atomic object that is added to it
 * and exactly one instance of each edge that is created by a call to connect().
 * Subsequent calls to add_atomic() for an Atomic model that is already in the
 * Graph will increase the count of instances of the model. The same happends for
 * edges created by connect(). When you remove edges or instances, the counts
 * are decreased by one for each removal. Only when the count reaches zero is
 * the instance of the edge or Atomic model physically removed from the Graph.
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
 * @see Coupled
*/
template <typename ValueType = std::any, typename TimeType = double>
class Graph {
    public:
        /**
         * @brief  Construct an empty graph.
         * 
         * The new Graph is in the non-provisional mode.
         */
        Graph():provisional(false){}
        /// @brief Destroy the graph but leave its Atomic components intact.
        virtual ~Graph(){}
        /**
         * @brief  Remove a pin.
         * 
         * All edges connected to this pin are removed from the graph.
         * @param p The pin to remove.
         */
        void remove_pin(pin_t p);
        /**
         * @brief Add an atomic model to the graph.
         * 
         * Each Atomic model has an instance count, which is the number
         * of times it has been added to the Graph. If the Atomic already
         * exists in the graph then this method simply increases its
         * instance count.
         * 
         * @see Coupled
         * 
         * @param model The model to add.
         */
        void add_atomic(std::shared_ptr<Atomic<ValueType,TimeType>> model);
        /**
         * @brief Remove an atomic model from the graph.
         * 
         * Reduces the instance count of the model. If the instance
         * count reaches zero then the model and all edges from a pin
         * to the model are removed from the graph.
         *
         *  @see Coupled  
         *
         *  @param model The model to remove.
         */
        void remove_atomic(std::shared_ptr<Atomic<ValueType, TimeType>> model);
        /**
         * @brief Connect two pins in the graph.
         * 
         * Create a link from the source pin the destination pin.
         * Any event placed on the source pin will be transmitted
         * to the destination pin.
         * 
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
        void connect(pin_t pin, std::shared_ptr<Atomic<ValueType, TimeType>> model);
        /**
         * @brief  Remove a connection from a pin and to an Atomic model.
         * 
         * This removes the link from the pin to the Atomic model.
         * @param pin The pin to disconnect.
         * @param model The model to disconnect.
         */
        void disconnect(pin_t pin, std::shared_ptr<Atomic<ValueType, TimeType>> model);
        /**
         * @brief  Get the Atomic models that are connected to a pin.
         * 
         * This fills the supplied list with Atomic model and pin pairs. The pairs
         * in the list are the pin to Atomic connections added with connect(pin_t, Atomic<ValueType,TimeType>).
         * The list contains just those pin and Atomic pairs for which their is a path
         * from the supplied pin.
         * @param pin The pin to query.
         * @param models A list to be filled with models and pins that are connected to the supplied pin.
         */
        void route(pin_t pin, std::list<std::pair<pin_t,std::shared_ptr<Atomic<ValueType, TimeType>>>>& models) const;
        /**
         *  @brief  Get the set of Atomic models that are part of the graph.
         * 
         * This returns a set of all Atomic models that have been added to the graph.
         * @return The set of all Atomic models.
         */
        const std::set<std::shared_ptr<Atomic<ValueType,TimeType>>>& get_atomics() const { return models; }
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
            /// @brief  A pending call to connect(pin_t, std::shared_ptr<Atomic<ValueType,TimeType>>).
            CONNECT_PIN_TO_ATOMIC,
            /// @brief  A pending call to disconnect(pin_t, std::shared_ptr<Atomic<ValueType,TimeType>>).
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
            std::shared_ptr<Atomic<ValueType,TimeType>> model;
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
        std::map<pin_t,std::list<std::pair<std::shared_ptr<Atomic<ValueType,TimeType>>,int>>> pin_to_atomic;
        std::map<pin_t,std::list<std::pair<pin_t,int>>> pin_to_pin;
        std::map<Atomic<ValueType,TimeType>*,int> atomic_instance_count;
        std::set<std::shared_ptr<Atomic<ValueType,TimeType>>> models;

        void queue_remove_pin(pin_t p);
        void queue_add_atomic(std::shared_ptr<Atomic<ValueType,TimeType>> model);
        void queue_remove_atomic(std::shared_ptr<Atomic<ValueType, TimeType>> model);
        void queue_connect(pin_t src, pin_t dst);
        void queue_disconnect(pin_t src, pin_t dst);
        void queue_connect(pin_t pin, std::shared_ptr<Atomic<ValueType, TimeType>> model);
        void queue_disconnect(pin_t pin, std::shared_ptr<Atomic<ValueType, TimeType>> model);

        // Pending changes are provisional? This is used by the simulator
        // to indicate that the graph is in a provisional state during execution.
        // When true, changes are queued but not applied to the graph.
        bool provisional;
        std::list<graph_op> pending;
    };

template <typename ValueType, typename TimeType>
void Graph<ValueType, TimeType>::remove_pin(pin_t pin) {
    if (provisional) {
        queue_remove_pin(pin);
        return;
    }
    // Remove atomic models from the pin to atomic map
    auto& atomic_list = pin_to_atomic[pin];
    auto pin_to_atomic_iter = atomic_list.begin();
    while (pin_to_atomic_iter != atomic_list.end()) {
        (*pin_to_atomic_iter).second--;
        if ((*pin_to_atomic_iter).second == 0) {
            pin_to_atomic_iter = atomic_list.erase(pin_to_atomic_iter);
        } else {
            pin_to_atomic_iter++;
        }
    }
    if (atomic_list.empty()) {
        // Remove the pin from the pin_to_atomic map.
        pin_to_atomic.erase(pin);
    }
    // Remove pins from the pin to pin map
    auto& pin_list = pin_to_pin[pin];
    auto pin_to_pin_iter = pin_list.begin();
    while (pin_to_pin_iter != pin_list.end()) {
        (*pin_to_pin_iter).second--;
        if ((*pin_to_pin_iter).second == 0) {
            pin_to_pin_iter = pin_list.erase(pin_to_pin_iter);
        } else {
            pin_to_pin_iter++;
        }
    }
    if (pin_list.empty()) {
        // Remove the pin from the pin_to_atomic map.
        pin_to_pin.erase(pin);
    }
}

template <typename ValueType, typename TimeType>
void Graph<ValueType, TimeType>::add_atomic(std::shared_ptr<Atomic<ValueType,TimeType>> model) {
    if (provisional) {
        queue_add_atomic(model);
        return;
    }
    auto instance_iter = atomic_instance_count.find(model.get());
    if (instance_iter != atomic_instance_count.end()) {
        // Increase the instance count and return.
        (*instance_iter).second++;
        return;
    } else {
        atomic_instance_count[model.get()] = 1;
    }
    // Add the model to the set of components map.
    models.insert(model);
}

template <typename ValueType, typename TimeType>
void Graph<ValueType, TimeType>::remove_atomic(std::shared_ptr<Atomic<ValueType,TimeType>> model) {
    if (provisional) {
        queue_remove_atomic(model);
        return;
    }
    auto instance_iter = atomic_instance_count.find(model.get());
    // Decrease the instance count and return.
    (*instance_iter).second--;
    if ((*instance_iter).second > 0) {
        // If there are still instances of the model, just return.
        return;
    }
    // Remove all pin couplings to the removed model
    atomic_instance_count.erase(instance_iter);
    for (auto i = pin_to_atomic.begin(); i != pin_to_atomic.end(); i++) {
        auto atomic_list_iter = i->second.begin();
        while (atomic_list_iter != i->second.end()) {
            if (atomic_list_iter->first == model) {
                // Remove the model from the pin to atomic map.
                atomic_list_iter = i->second.erase(atomic_list_iter);
            } else {
                atomic_list_iter++;
            }
        }
    }
    models.erase(model);
}

template <typename ValueType, typename TimeType>
void Graph<ValueType, TimeType>::connect(pin_t src, pin_t dst) {
    if (provisional) {
        queue_connect(src, dst);
        return;
    }
    // If this connection already exists, increase the instance count and return
    std::list<std::pair<pin_t,int>>& pin_list = pin_to_pin[src];
    auto iter = pin_list.begin();
    while (iter != pin_list.end()) {
        if ((*iter).first == dst) {
            // Increase the instance count and return.
            (*iter).second++;
            return;
        }
        iter++;
    }
    // Didn't find it. Add the connection.
    pin_list.push_back(std::pair<pin_t,int>(dst,1));
}

template <typename ValueType, typename TimeType>
void Graph<ValueType, TimeType>::disconnect(pin_t src, pin_t dst) {
    if (provisional) {
        queue_disconnect(src, dst);
        return;
    }
    auto& pin_list = pin_to_pin[src];
    auto iter = pin_list.begin();
    while (iter != pin_list.end()) {
        if ((*iter).first == dst) {
            // Decrease the instance count
            (*iter).second--;
            // Remove it if it is the last instance
            if ((*iter).second == 0) {
                pin_list.erase(iter);
                if (pin_list.empty()) {
                    // If the pin_to_pin map is empty, remove the pin.
                    pin_to_pin.erase(src);
                }
            } 
            // Done so return
            return;
        }
        iter++;
    }
}

template <typename ValueType, typename TimeType>
void Graph<ValueType, TimeType>::connect(pin_t pin, std::shared_ptr<Atomic<ValueType,TimeType>> model) {
    if (provisional) {
        queue_connect(pin, model);
        return;
    }
    auto& atomic_list = pin_to_atomic[pin];
    auto iter = atomic_list.begin();
    while (iter != atomic_list.end()) {
        if ((*iter).first == model) {
            // Increase the instance count and return.
            (*iter).second++;
            return;
        }
        iter++;
    }
    // Add the connection to the pin_to_atomic map.
    pin_to_atomic[pin].push_back(std::pair<std::shared_ptr<Atomic<ValueType,TimeType>>,int>(model,1));
}

template <typename ValueType, typename TimeType>
void Graph<ValueType, TimeType>::disconnect(pin_t pin, std::shared_ptr<Atomic<ValueType,TimeType>> model) {
    if (provisional) {
        queue_disconnect(pin, model);
        return;
    }
    auto& atomic_list = pin_to_atomic[pin];
    auto iter = atomic_list.begin();
    while (iter != atomic_list.end()) {
        if ((*iter).first == model) {
            (*iter).second--;
            if ((*iter).second == 0) {
                // Remove the model from the pin to atomic map.
                atomic_list.erase(iter);
                if (atomic_list.empty()) {
                    // If the pin_to_atomic map is empty, remove the pin.
                    pin_to_atomic.erase(pin);
                }
            }
            return;
        }
        iter++;
    }
}

template <typename ValueType, typename TimeType>
void Graph<ValueType,TimeType>::route(pin_t pin, std::list<std::pair<pin_t,std::shared_ptr<Atomic<ValueType, TimeType>>>>& models) const {
    auto i = pin_to_atomic.find(pin);
    if (i != pin_to_atomic.end()) {
        for (auto j = i->second.begin(); j != i->second.end(); j++) {
            models.push_back(std::pair<pin_t,std::shared_ptr<Atomic<ValueType, TimeType>>>(pin,(*j).first));
        }
    }
    auto r = pin_to_pin.find(pin);
    if (r != pin_to_pin.end()) {
        for (auto s = r->second.begin(); s != r->second.end(); s++) {
            route((*s).first, models);
        }
    }
}

template <typename ValueType, typename TimeType>
void Graph<ValueType, TimeType>::queue_add_atomic(std::shared_ptr<Atomic<ValueType,TimeType>> model) {
    graph_op op;
    op.op = ADD_ATOMIC;
    op.model = model;
    pending.push_back(op);
}

template <typename ValueType, typename TimeType>
void Graph<ValueType, TimeType>::queue_remove_pin(pin_t pin) {
    graph_op op;
    op.op = REMOVE_PIN;
    op.pin[0] = pin;
    pending.push_back(op);
}

template <typename ValueType, typename TimeType>
void Graph<ValueType, TimeType>::queue_remove_atomic(std::shared_ptr<Atomic<ValueType,TimeType>> model) {
    graph_op op;
    op.op = REMOVE_ATOMIC;
    op.model = model;
    pending.push_back(op);
}

template <typename ValueType, typename TimeType>
void Graph<ValueType, TimeType>::queue_connect(pin_t src, pin_t dst) {
    graph_op op;
    op.op = CONNECT_PIN_TO_PIN;
    op.pin[0] = src;
    op.pin[1] = dst;
    pending.push_back(op);
}

template <typename ValueType, typename TimeType>
void Graph<ValueType, TimeType>::queue_disconnect(pin_t src, pin_t dst) {
    graph_op op;
    op.op = DISCONNECT_PIN_FROM_PIN;
    op.pin[0] = src;
    op.pin[1] = dst;
    pending.push_back(op);
}

template <typename ValueType, typename TimeType>
void Graph<ValueType, TimeType>::queue_connect(pin_t pin, std::shared_ptr<Atomic<ValueType,TimeType>> model) {
    graph_op op;
    op.op = CONNECT_PIN_TO_ATOMIC;
    op.pin[0] = pin;
    op.model = model;
    pending.push_back(op);
}

template <typename ValueType, typename TimeType>
void Graph<ValueType, TimeType>::queue_disconnect(pin_t pin, std::shared_ptr<Atomic<ValueType,TimeType>> model) {
    graph_op op;
    op.op = DISCONNECT_PIN_FROM_ATOMIC;
    op.pin[0] = pin;
    op.model = model;
    pending.push_back(op);
}

}

#endif
