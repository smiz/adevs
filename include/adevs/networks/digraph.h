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

#ifndef _adevs_digraph_h_
#define _adevs_digraph_h_

#include <cassert>
#include <cstdlib>
#include <map>
#include <set>
#include "adevs/adevs.h"


namespace adevs {

/*
 * The components of a digraph model must use PortValue objects
 * as their basic I/O type: the port and value types are template
 * arguments. The default port type is an integer.
 */
template <class VALUE, class PORT = int>
class PortValue {
  public:
    /// Constructor
    PortValue() : port(), value() {}
    /// Copy constructor
    PortValue(PortValue const &src) : port(src.port), value(src.value) {}
    /// Create an object with the specified port and value
    PortValue(PORT port, const VALUE &value) : port(port), value(value) {}
    /// Assignment operator
    PortValue<VALUE, PORT> const &operator=(PortValue<VALUE, PORT> const &src) {
        port = src.port;
        value = src.value;
        return *this;
    }
    /// Destructor
    ~PortValue() {}
    /// The port on which the value appears
    PORT port;
    /// The value appearing on the port
    VALUE value;
};

/*
 * The digraph model is used to build block-diagrams from network and atomic components.
 * Its components must have PortValue objects as their input/output type.
 */
template <class VALUE, class PORT = int, class TimeType = double>
class Digraph : public Network<PortValue<VALUE, PORT>, TimeType> {
  public:
    /// An input or output to a component model
    typedef PortValue<VALUE, PORT> IO_Type;
    /// A component of the Digraph model
    typedef Devs<IO_Type, TimeType> Component;

    /// Construct a network with no components.
    Digraph() : Network<IO_Type, TimeType>() {}
    /// Add a model to the network.
    void add(shared_ptr<Component> model);
    /// Couple the source model to the destination model.
    void couple(shared_ptr<Component> src, PORT srcPort,
                shared_ptr<Component> dst, PORT dstPort);
    // Required due to switching to smart pointers for components instead of raw pointers.
    // This hopefully won't be needed once the networks are refactored in v4.0.
    void couple_input(PORT srcPort, shared_ptr<Component> dst, PORT dstPort);
    void couple_output(shared_ptr<Component> src, PORT srcPort, PORT dstPort);

    /// Puts the network's components into to c
    void getComponents(set<Component*> &c);
    /// Route an event based on the coupling information.
    void route(IO_Type const &x, Component* model,
               list<Event<IO_Type, TimeType>> &r);

  private:
    // A node in the coupling graph
    struct node {
        node() : model(nullptr), port() {}
        node(Component* model, PORT port) : model(model), port(port) {}
        node const &operator=(node const &src) {
            model = src.model;
            port = src.port;
            return *this;
        }
        Component* model;
        PORT port;

        // Comparison for STL map
        bool operator<(node const &other) const {
            if (model == other.model) {
                return port < other.port;
            }
            return model < other.model;
        }
    };
    // Component model set
    set<shared_ptr<Component>> models;
    // Coupling information
    std::map<node, list<node>> graph;
};

template <class VALUE, class PORT, class TimeType>
void Digraph<VALUE, PORT, TimeType>::add(shared_ptr<Component> model) {
    assert(model.get() != this);
    models.insert(model);
    model->setParent(this);
    if (this->simulator != nullptr) {
        this->simulator->pending_schedule.insert(model);
    }
}

template <class VALUE, class PORT, class TimeType>
void Digraph<VALUE, PORT, TimeType>::couple(shared_ptr<Component> src,
                                            PORT srcPort,
                                            shared_ptr<Component> dst,
                                            PORT dstPort) {
    if (src.get() != this) {
        add(src);
    }
    if (dst.get() != this) {
        add(dst);
    }
    node src_node(src.get(), srcPort);
    node dst_node(dst.get(), dstPort);
    graph[src_node].push_back(dst_node);
}

template <class VALUE, class PORT, class TimeType>
void Digraph<VALUE, PORT, TimeType>::couple_input(PORT srcPort,
                                                  shared_ptr<Component> dst,
                                                  PORT dstPort) {
    add(dst);
    node src_node(this, srcPort);
    node dst_node(dst.get(), dstPort);
    graph[src_node].push_back(dst_node);
}

template <class VALUE, class PORT, class TimeType>
void Digraph<VALUE, PORT, TimeType>::couple_output(shared_ptr<Component> src,
                                                   PORT srcPort, PORT dstPort) {
    add(src);
    node src_node(src.get(), srcPort);
    node dst_node(this, dstPort);
    graph[src_node].push_back(dst_node);
}

template <class VALUE, class PORT, class TimeType>
void Digraph<VALUE, PORT, TimeType>::getComponents(set<Component*> &c) {
    for (auto ii : models) {
        c.insert(ii.get());
    }
}

template <class VALUE, class PORT, class TimeType>
void Digraph<VALUE, PORT, TimeType>::route(IO_Type const &x, Component* model,
                                           list<Event<IO_Type, TimeType>> &r) {
    // Find the list of target models and ports
    node src_node(model, x.port);
    typename std::map<node, list<node>>::iterator graph_iter;
    graph_iter = graph.find(src_node);
    // If no target, just return
    if (graph_iter == graph.end()) {
        return;
    }
    // Otherwise, add the targets to the event list
    Event<IO_Type, TimeType> event;
    for (auto node_iter : (*graph_iter).second) {
        event.model = node_iter.model;
        event.value.port = node_iter.port;
        event.value.value = x.value;
        r.push_back(event);
    }
}

}  // namespace adevs

#endif
