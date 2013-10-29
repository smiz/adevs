/**
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
#ifndef __adevs_digraph_h_
#define __adevs_digraph_h_
#include "adevs.h"
#include <map>
#include <set>
#include <cstdlib>

namespace adevs
{

/**
 * The components of a digraph model must use PortValue objects
 * as their basic I/O type: the port and value types are template
 * arguments. The default port type is an integer.
 */
template <class VALUE, class PORT=int> class PortValue
{
	public:
		/// Constructor
		PortValue():
		port(),
		value()
		{
		}
		/// Copy constructor
		PortValue(const PortValue& src):
		port(src.port),
		value(src.value)
		{
		}
		/// Create an object with the specified port and value
		PortValue(PORT port, const VALUE& value):
		port(port),
		value(value)
		{
		}
		/// Assignment operator
		const PortValue<VALUE,PORT>& operator=(const PortValue<VALUE,PORT>& src)
		{
			port = src.port;
			value = src.value;
			return *this;
		}
		/// Destructor
		~PortValue()
		{
		}
		/// The port on which the value appears
		PORT port;
		/// The value appearing on the port
		VALUE value;
};

/**
 * The digraph model is used to build block-diagrams from network and atomic components.
 * Its components must have PortValue objects as their input/output type.
 */
template <class VALUE, class PORT=int, class T = double> class Digraph: 
public Network<PortValue<VALUE,PORT>,T>
{
	public:
		/// An input or output to a component model
		typedef PortValue<VALUE,PORT> IO_Type;
		/// A component of the Digraph model
		typedef Devs<IO_Type,T> Component;

		/// Construct a network with no components.
		Digraph():
		Network<IO_Type,T>()
		{
		}
		/// Add a model to the network.
		void add(Component* model);
		/// Couple the source model to the destination model.  
		void couple(Component* src, PORT srcPort, 
		Component* dst, PORT dstPort);
		/// Puts the network's components into to c
		void getComponents(Set<Component*>& c);
		/// Route an event based on the coupling information.
		void route(const IO_Type& x, Component* model, 
		Bag<Event<IO_Type,T> >& r);
		/// Destructor.  Destroys all of the component models.
		~Digraph();

	private:	
		// A node in the coupling graph
		struct node
		{
			node():
			model(NULL),
			port()
			{
			}
			node(Component* model, PORT port):
			model(model),
			port(port)
			{
			}
			const node& operator=(const node& src)
			{
				model = src.model;
				port = src.port;
				return *this;
			}
			Component* model;
			PORT port;
		
			// Comparison for STL map
			bool operator<(const node& other) const
			{
				if (model == other.model) return port < other.port;
				return model < other.model;
			}
		};
		// Component model set
		Set<Component*> models;
		// Coupling information
		std::map<node,Bag<node> > graph;
};

template <class VALUE, class PORT, class T>
void Digraph<VALUE,PORT,T>::add(Component* model)
{
	assert(model != this);
	models.insert(model);
	model->setParent(this);
}

template <class VALUE, class PORT, class T>
void Digraph<VALUE,PORT,T>::couple(Component* src, PORT srcPort, 
Component* dst, PORT dstPort)
{
	if (src != this) add(src);
	if (dst != this) add(dst);
	node src_node(src,srcPort);
	node dst_node(dst,dstPort);
	graph[src_node].insert(dst_node);
}

template <class VALUE, class PORT, class T>
void Digraph<VALUE,PORT,T>::getComponents(Set<Component*>& c)
{
	c = models;
}

template <class VALUE, class PORT, class T>
void Digraph<VALUE,PORT,T>::
route(const IO_Type& x, Component* model, 
Bag<Event<IO_Type,T> >& r)
{
	// Find the list of target models and ports
	node src_node(model,x.port);
	typename std::map<node,Bag<node> >::iterator graph_iter;
	graph_iter = graph.find(src_node);
	// If no target, just return
	if (graph_iter == graph.end()) return;
	// Otherwise, add the targets to the event bag
	Event<IO_Type,T> event;
	typename Bag<node>::iterator node_iter;
	for (node_iter = (*graph_iter).second.begin();
	node_iter != (*graph_iter).second.end(); node_iter++)
	{
		event.model = (*node_iter).model;
		event.value.port = (*node_iter).port;
		event.value.value = x.value;
		r.insert(event);
	}
}
template <class VALUE, class PORT, class T>
Digraph<VALUE,PORT,T>::~Digraph()
{ 
	typename Set<Component*>::iterator i;
	for (i = models.begin(); i != models.end(); i++)
	{
		delete *i;
	}
}

} // end of namespace 

#endif
