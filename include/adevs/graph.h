
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

template <typename X, typename T = double>
class Graph {
    public:
        /// @brief  Construct an empty graph.
        Graph():next_pin(0) {}
        /// @brief  Destroy the graph but leave its atomic components intact.
        virtual ~Graph(){}
        /// @brief  Add a new pin_t to the graph.
        /// @return The pin that was created.
        pin_t add_pin() { return next_pin++; };
        /// @brief  Remove a pin_t from the graph.
        /// @param p The pin_t to remove.
        void remove_pin(pin_t p);
        /// @brief Add an atomic model to the graph.
        /// @param model The model to add.
        void add_atomic(std::shared_ptr<Atomic<X,T>> model) { models.insert(model); }
        /// @brief Remove an atomic model from the graph.
        /// @param model The model to remove.
        void remove_atomic(std::shared_ptr<Atomic<X, T>> model);
        /// Connect two pins in the graph. Events appearing on the source pin
        /// will be delivered to the destination pin.
        /// @param src The source pin.
        /// @param dst The destination pin.
        void connect(pin_t src, pin_t dst);
        /// @brief  Remove a connection between two pins.
        /// @param src The source pin.
        /// @param dst The destination pin.
        void disconnect(pin_t src, pin_t dst);
        /// @brief  Connect a pin_t to an atomic model. Events appearing on the pin
        /// will be delivered to the atomic model as input.
        /// @param pin The pin to connect.
        /// @param model The model to receive the input.
        void connect(pin_t pin, std::shared_ptr<Atomic<X, T>> model);
        /// @brief  Remove a connection between a pin and an atomic model.
        /// @param pin The pin to disconnect.
        /// @param model The model to disconnect.
        void disconnect(pin_t pin, std::shared_ptr<Atomic<X, T>> model);
        /// @brief  Get the atomic models that are connected to a pin_t. These
        /// are the models that will receive input when an event appears on the pin_t.
        /// @param pin The pin to query.
        /// @param models A list to be filled with models and pins that are connected to the supplied pin.
        void route(pin_t pin, std::list<std::pair<pin_t,std::shared_ptr<Atomic<X, T>>>>& models) const;
        /// @brief  Get the set of all atomic models that are part of the graph.
        /// @return The set of all atomic models.
        const std::set<std::shared_ptr<Atomic<X,T>>>& get_atomics() const { return models; }
    private:
        std::map<pin_t,std::list<std::shared_ptr<Atomic<X,T>>>> pin_to_atomic;
        std::map<pin_t,std::list<pin_t> > pin_to_pin;
        std::set<std::shared_ptr<Atomic<X,T>>> models;
        pin_t next_pin;
    };

template <typename X, typename T>
void Graph<X, T>::remove_pin(pin_t pin) {
    pin_to_atomic.erase(pin);
    pin_to_pin.erase(pin);
    for (auto i = pin_to_pin.begin(); i != pin_to_pin.end(); i++) {
        i->second.remove(pin);
    }
}

template <typename X, typename T>
void Graph<X, T>::remove_atomic(std::shared_ptr<Atomic<X,T>> model) {
    for (auto i = pin_to_atomic.begin(); i != pin_to_atomic.end(); i++) {
        i->second.remove(model);
    }
    models.erase(model);
}

template <typename X, typename T>
void Graph<X, T>::connect(pin_t src, pin_t dst) {
    pin_to_pin[src].push_back(dst);
}

template <typename X, typename T>
void Graph<X, T>::disconnect(pin_t src, pin_t dst) {
    auto i = pin_to_pin.find(src);
    if (i != pin_to_pin.end()) {
        i->second.remove(dst);
    }
}

template <typename X, typename T>
void Graph<X, T>::connect(pin_t pin, std::shared_ptr<Atomic<X,T>> model) {
    models.insert(model);
    pin_to_atomic[pin].push_back(model);
}

template <typename X, typename T>
void Graph<X, T>::disconnect(pin_t pin, std::shared_ptr<Atomic<X,T>> model) {
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

}

#endif
