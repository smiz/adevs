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
#ifndef __adevs_simpledigraph_h_
#define __adevs_simpledigraph_h_
#include "adevs.h"
#include <map>
#include <set>
#include <cstdlib>

namespace adevs
{

/**
 * This is a very simple digraph model for single input/single output
 * models. Output events generated by a component model are sent to all
 * connected models.
 */
template <class VALUE> class SimpleDigraph: 
public Network<VALUE>
{
	public:
		/// A component of the SimpleDigraph model
		typedef Devs<VALUE> Component;

		/// Construct a network with no components.
		SimpleDigraph():
		Network<VALUE>()
		{
		}
		/// Add a model to the network.
		void add(Component* model);
		/// Couple the source model to the destination model.  
		void couple(Component* src, Component* dst);
		/// Puts the network's set of components into c
		void getComponents(Set<Component*>& c);
		/// Route an event according to the network's couplings
		void route(const VALUE& x, Component* model, 
		Bag<Event<VALUE> >& r);
		/// Destructor.  Destroys all of the component models.
		~SimpleDigraph();

	private:	
		// Component model set
		Set<Component*> models;
		// Coupling information
		std::map<Component*,Bag<Component*> > graph;
};

template <class VALUE>
void SimpleDigraph<VALUE>::add(Component* model)
{
	assert(model != this);
	models.insert(model);
	model->setParent(this);
}

template <class VALUE>
void SimpleDigraph<VALUE>::couple(Component* src, Component* dst) 
{
	if (src != this) add(src);
	if (dst != this) add(dst);
	graph[src].insert(dst);
}

template <class VALUE>
void SimpleDigraph<VALUE>::getComponents(Set<Component*>& c)
{
	c = models;
}

template <class VALUE>
void SimpleDigraph<VALUE>::
route(const VALUE& x, Component* model, 
Bag<Event<VALUE> >& r)
{
	// Find the list of target models and ports
	typename std::map<Component*,Bag<Component*> >::iterator graph_iter;
	graph_iter = graph.find(model);
	// If no target, just return
	if (graph_iter == graph.end()) return;
	// Otherwise, add the targets to the event bag
	Event<VALUE> event;
	typename Bag<Component*>::iterator node_iter;
	for (node_iter = (*graph_iter).second.begin();
	node_iter != (*graph_iter).second.end(); node_iter++)
	{
		event.model = *node_iter;
		event.value = x;
		r.insert(event);
	}
}

template <class VALUE>
SimpleDigraph<VALUE>::~SimpleDigraph()
{ 
	typename Set<Component*>::iterator i;
	for (i = models.begin(); i != models.end(); i++)
	{
		delete *i;
	}
}

} // end of namespace 

#endif
