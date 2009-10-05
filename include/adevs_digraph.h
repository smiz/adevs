/***************
Copyright (C) 2000-2006 by James Nutaro

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Bugs, comments, and questions can be sent to nutaro@gmail.com
***************/
#ifndef __adevs_digraph_h_
#define __adevs_digraph_h_
#include "adevs.h"
#include <map>
#include <set>
#include <cstdlib>

namespace adevs
{

/**
 * The digraph model requires components to use PortValue objects
 * as their basic I/O type.  The port and value types are template
 * arguments.  The default port type is an integer.
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
 * This Digraph model uses PortValue objects to describe
 * component coupling.  The default port type is an integer.
 */
template <class VALUE, class PORT=int> class Digraph: 
public Network<PortValue<VALUE,PORT> >
{
	public:
		/// A component input or output
		typedef PortValue<VALUE,PORT> IO_Type;
		/// A component of the Digraph model
		typedef Devs<IO_Type> Component;

		/// Construct a network with no components.
		Digraph():
		Network<IO_Type>()
		{
		}
		/// Add a model to the network.
		void add(Component* model);
		/// Couple the source model to the destination model.  
		void couple(Component* src, PORT srcPort, 
		Component* dst, PORT dstPort);
		/// Assigns the model component set to c
		void getComponents(Set<Component*>& c);
		/// Route an event based on the coupling information.
		void route(const IO_Type& x, Component* model, 
		Bag<Event<IO_Type> >& r);
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

template <class VALUE, class PORT>
void Digraph<VALUE,PORT>::add(Component* model)
{
	assert(model != this);
	models.insert(model);
	model->setParent(this);
}

template <class VALUE, class PORT>
void Digraph<VALUE,PORT>::couple(Component* src, PORT srcPort, 
Component* dst, PORT dstPort)
{
	if (src != this) add(src);
	if (dst != this) add(dst);
	node src_node(src,srcPort);
	node dst_node(dst,dstPort);
	graph[src_node].insert(dst_node);
}

template <class VALUE, class PORT>
void Digraph<VALUE,PORT>::getComponents(Set<Component*>& c)
{
	c = models;
}

template <class VALUE, class PORT>
void Digraph<VALUE,PORT>::
route(const IO_Type& x, Component* model, 
Bag<Event<IO_Type> >& r)
{
	// Find the list of target models and ports
	node src_node(model,x.port);
	typename std::map<node,Bag<node> >::iterator graph_iter;
	graph_iter = graph.find(src_node);
	// If no target, just return
	if (graph_iter == graph.end()) return;
	// Otherwise, add the targets to the event bag
	Event<IO_Type> event;
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
template <class VALUE, class PORT>
Digraph<VALUE,PORT>::~Digraph()
{ 
	typename Set<Component*>::iterator i;
	for (i = models.begin(); i != models.end(); i++)
	{
		delete *i;
	}
}

} // end of namespace 

#endif
