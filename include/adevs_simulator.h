/***************
Copyright (C) 2000-2008 by James Nutaro

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
#ifndef __adevs_simulator_h_
#define __adevs_simulator_h_
#include "adevs_abstract_simulator.h"
#include "adevs_models.h"
#include "adevs_event_listener.h"
#include "adevs_sched.h"
#include "adevs_bag.h"
#include "adevs_set.h"
#include "object_pool.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace adevs
{

/**
 * This Simulator class implements the DEVS simulation algorithm.
 * Its methods can throw adevs::exception objects if any of the DEVS model
 * constraints are violated (i.e., a negative time advance of a model
 * attempting to send an input directly to itself).
 */
template <class X> class Simulator:
	public AbstractSimulator<X>
{
	public:
		/**
		Create a simulator for the provided model. The simulator
		constructor will fail and throw an adevs::exception if the
		time advance of any component atomic model is less than zero.
		*/
		Simulator(Devs<X>* model):
			AbstractSimulator<X>()
		{
			schedule(model,0.0);
		}
		/// Get the model's next event time
		double nextEventTime()
		{
			return sched.minPriority();
		}
		/// Execute the simulation cycle at time nextEventTime()
		void execNextEvent()
		{
			computeNextOutput();
			computeNextState(bogus_input,sched.minPriority());
		}
		/// Execute until nextEventTime() > tend
		void execUntil(double tend)
		{
			while (nextEventTime() <= tend && nextEventTime() < DBL_MAX)
				execNextEvent();
		}
		/**
		Compute the output values of the imminent models if these values
		have not already been computed.  This will
		notify the EventListener objects as the outputs are found. This,
		in effect, implements the output function of the resultant
		model.
		*/
		void computeNextOutput();
		/**
		Apply the specified inputs at time t and then compute the next model
		states.  Requires that lastEventTime() <= t <= nextEventTime().
		This, in effect, implements the state transition function of the resultant
		model.
		*/
		void computeNextState(Bag<Event<X> >& input, double t);
		/**
		Deletes the simulator, but leaves the model intact. The model must
		exist when the simulator is deleted.  Delete the model only after
		the Simulator is deleted.
		*/
		~Simulator();
	private:
		/// Bogus input bag for execNextEvent() method
		Bag<Event<X> > bogus_input;
		/// The event schedule
		Schedule<X> sched;
		/// List of imminent models
		Bag<Atomic<X>*> imm;
		/// List of models activated by input
		Bag<Atomic<X>*> activated;
		/// Pools of preallocated, commonly used objects
		object_pool<Bag<X> > io_pool;
		object_pool<Bag<Event<X> > > recv_pool;
		// Sets for computing structure changes.
		Set<Devs<X>*> added;
		Set<Devs<X>*> removed;
		Set<Devs<X>*> next;
		Set<Devs<X>*> prev;
		// Model transition functions are evaluated from the bottom up!
		struct bottom_to_top_depth_compare
		{
			bool operator()(const Network<X>* m1, const Network<X>* m2) const
			{
				unsigned long int d1 = 0, d2 = 0;
				// Compute depth of m1
				const Network<X>* m = m1->getParent();
				while (m != NULL) 
				{
					d1++;
					m = m->getParent();
				}
				// Compute depth of m2
				m = m2->getParent();
				while (m != NULL)
				{
					d2++;
					m = m->getParent();
				}
				// Models at the same depth are sorted by name
				if (d1 == d2) return m1 < m2;
				// Otherwise, sort by depth
				return d1 > d2;
			}
		};
		struct top_to_bottom_depth_compare
		{
			bool operator()(const Devs<X>* m1, const Devs<X>* m2) const
			{
				unsigned long int d1 = 0, d2 = 0;
				// Compute depth of m1
				const Network<X>* m = m1->getParent();
				while (m != NULL) 
				{
					d1++;
					m = m->getParent();
				}
				// Compute depth of m2
				m = m2->getParent();
				while (m != NULL)
				{
					d2++;
					m = m->getParent();
				}
				// Models at the same depth are sorted by name
				if (d1 == d2) return m1 < m2;
				// Otherwise, sort by depth
				return d1 < d2;
			}
		};
		std::set<Network<X>*,bottom_to_top_depth_compare> model_func_eval_set;
		std::set<Devs<X>*,top_to_bottom_depth_compare> sorted_removed;
		/**
		Recursively add the model and its elements to the schedule using t as the time of
		last event.
		*/
		void schedule(Devs<X>* model, double t);
		/// Route an event generated by the source model contained in the parent model.
		void route(Network<X>* parent, Devs<X>* src, X& x);
		/**	
		Add an input to the input bag of an an atomic model. If the model's active flag is false,
		this method adds the model to the activated bag and sets the active flag to true.
		*/
		void inject_event(Atomic<X>* model, X& value);
		/**
		Recursively remove a model and its components from the schedule 
		and the imminent/activated bags
		*/
		void unschedule_model(Devs<X>* model);
		/**
		Set the atomic model active flag to false, delete any thing in the output bag,
		and return the input and output bags to the pools.  Recursively clean up
		network model components.
		*/
		void clean_up(Devs<X>* model);
		/**
		Execute the state transition function using t to compute the elapsed time as t-model->tL.
		This adds the model to the nx bag if it is a network executive and updates the added
		and removed sets. 
		*/
		void exec_event(Atomic<X>* model, bool internal, double t);
		/**
		Construct the complete descendent set of a network model and store it in s.
		*/
		void getAllChildren(Network<X>* model, Set<Devs<X>*>& s);
};

template <class X>
void Simulator<X>::computeNextOutput()
{
	// If the imminent set is up to date, then just return
	if (imm.empty() == false) return;
	// Get the imminent models from the schedule. This sets the active flags.
	sched.getImminent(imm);
	// Compute output functions and route the events. The bags of output
	// are held for garbage collection at a later time.
	for (typename Bag<Atomic<X>*>::iterator imm_iter = imm.begin(); 
		imm_iter != imm.end(); imm_iter++)
	{
		Atomic<X>* model = *imm_iter;
		// If the output for this model has already been computed, then skip it
		if (model->y == NULL)
		{
			model->y = io_pool.make_obj();
			model->output_func(*(model->y));
			// Route each event in y
			for (typename Bag<X>::iterator y_iter = model->y->begin(); 
			y_iter != model->y->end(); y_iter++)
			{
				route(model->getParent(),model,*y_iter);
			}
		}
	}
}

template <class X>
void Simulator<X>::computeNextState(Bag<Event<X> >& input, double t)
{
	// Clean up if there was a previous IO calculation
	if (t < sched.minPriority())
	{
		typename Bag<Atomic<X>*>::iterator iter;
		for (iter = activated.begin(); iter != activated.end(); iter++)
		{
			clean_up(*iter);
		}
		activated.clear();
		for (iter = imm.begin(); iter != imm.end(); iter++)
		{
			clean_up(*iter);
		}
		imm.clear();
	}
	// Otherwise, if the internal IO needs to be computed, do it
	else if (t == sched.minPriority() && imm.empty())
	{
		computeNextOutput();
	}
	// Apply the injected inputs
	for (typename Bag<Event<X> >::iterator iter = input.begin(); 
	iter != input.end(); iter++)
	{
		Atomic<X>* amodel = (*iter).model->typeIsAtomic();
		if (amodel != NULL)
		{
			inject_event(amodel,(*iter).value);
		}
		else
		{
			route((*iter).model->typeIsNetwork(),(*iter).model,(*iter).value);
		}
	}
	/*
	Compute the states of atomic models.  Store Network models that need to have
	their model transition function evaluated in a
	special container that will be used when the structure changes are
	computed (see exec_event(.)).
	*/
	for (typename Bag<Atomic<X>*>::iterator iter = imm.begin(); 
	iter != imm.end(); iter++)
	{
		exec_event(*iter,true,t); // Internal and confluent transitions
	}
	for (typename Bag<Atomic<X>*>::iterator iter = activated.begin(); 
	iter != activated.end(); iter++)
	{
		exec_event(*iter,false,t); // External transitions
	}
	/**
	 * Compute model transitions and build up the prev (pre-transition)
	 * and next (post-transition) componenent sets. These sets are built
	 * up from only the models that have the model_transition function
	 * evalauted.
	 */
	if (model_func_eval_set.empty() == false)
	{
		while (!model_func_eval_set.empty())
		{
			Network<X>* network_model = *(model_func_eval_set.begin());
			getAllChildren(network_model,prev);
			if (network_model->model_transition() && network_model->getParent() != NULL)
			{
				model_func_eval_set.insert(network_model->getParent());
			}
			getAllChildren(network_model,next);
			model_func_eval_set.erase(network_model);
		}
		// Find the set of models that were added.
		set_assign_union(added,set_difference(next,prev));
		// Find the set of models that were removed
		set_assign_union(removed,set_difference(prev,next));
		next.clear();
		prev.clear();
		// Any models that moved from one network to another should be left alone.
		Set<Devs<X>*> moved(set_intersect(removed,added));
		for (typename Set<Devs<X>*>::iterator iter = moved.begin();
		iter != moved.end(); iter++)
		{
			typename Set<Devs<X>*>::iterator iter_tmp = added.find(*iter);
			if (iter_tmp != added.end()) added.erase(iter_tmp);
			iter_tmp = removed.find(*iter);
			if (iter_tmp != removed.end()) removed.erase(iter_tmp);
		}
		/** 
		 * The model adds are processed first.  This is done so that, if any
		 * of the added models are components something that was removed at
		 * a higher level, then the models will not have been deleted when
		 * trying to schedule them.
		 */
		for (typename Set<Devs<X>*>::iterator iter = added.begin(); 
			iter != added.end(); iter++)
		{
			schedule(*iter,t);
		}
		// Done with the additions
		added.clear();
		// Remove the models that are in the removed set.
		for (typename Set<Devs<X>*>::iterator iter = removed.begin(); 
			iter != removed.end(); iter++)
		{
			clean_up(*iter);
			unschedule_model(*iter);
			sorted_removed.insert(*iter); // Add to a sorted remove set for deletion
		}
		// Done with the unsorted remove set
		removed.clear();
		// Delete the sorted removed models
		while (!sorted_removed.empty())
		{
			// Get the model to erase
			Devs<X>* model_to_remove = *(sorted_removed.begin());
			/**
			 * If this model has children, then remove them from the deletion set.
			 * This will avoid double delete problems.
			 */
			if (model_to_remove->typeIsNetwork() != NULL)
			{
				getAllChildren(model_to_remove->typeIsNetwork(),removed);
				for (typename Set<Devs<X>*>::iterator iter = removed.begin(); 
					iter != removed.end(); iter++)
				{
					sorted_removed.erase(*iter);
				}
				removed.clear();
			}
			// Remove the model
			sorted_removed.erase(sorted_removed.begin());
			// Delete the model and its children
			delete model_to_remove;
		}
		// Removed sets should be empty now
		assert(removed.empty());
		assert(sorted_removed.empty());
	} // End of the structure change
	// Cleanup and reschedule models that changed state in this iteration
	// and survived the structure change phase.
	for (typename Bag<Atomic<X>*>::iterator iter = imm.begin(); // Schedule the imminents
	iter != imm.end(); iter++)
	{
		clean_up(*iter);
		schedule(*iter,t);
	}
	for (typename Bag<Atomic<X>*>::iterator iter = activated.begin(); // Schedule the activated
	iter != activated.end(); iter++)
	{
		clean_up(*iter);
		schedule(*iter,t);
	}
	// Empty the bags
	imm.clear();
	activated.clear();
}

template <class X>
void Simulator<X>::clean_up(Devs<X>* model)
{
	Atomic<X>* amodel = model->typeIsAtomic();
	if (amodel != NULL)
	{
		amodel->active = false;
		if (amodel->x != NULL)
		{
			amodel->x->clear();
			io_pool.destroy_obj(amodel->x);
			amodel->x = NULL;
		}
		if (amodel->y != NULL)
		{
			amodel->gc_output(*(amodel->y));
			amodel->y->clear();
			io_pool.destroy_obj(amodel->y);
			amodel->y = NULL;
		}
	}
	else
	{
		Set<Devs<X>*> components;
		model->typeIsNetwork()->getComponents(components);
		for (typename Set<Devs<X>*>::iterator iter = components.begin();
		iter != components.end(); iter++)
		{
			clean_up(*iter);
		}
	}
}

template <class X>
void Simulator<X>::unschedule_model(Devs<X>* model)
{
	if (model->typeIsAtomic() != NULL)
	{
		sched.schedule(model->typeIsAtomic(),DBL_MAX);
		imm.erase(model->typeIsAtomic());
		activated.erase(model->typeIsAtomic());
	}
	else
	{
		Set<Devs<X>*> components;
		model->typeIsNetwork()->getComponents(components);
		for (typename Set<Devs<X>*>::iterator iter = components.begin();
		iter != components.end(); iter++)
		{
			unschedule_model(*iter);
		}
	}
}

template <class X>
void Simulator<X>::schedule(Devs<X>* model, double t)
{
	Atomic<X>* a = model->typeIsAtomic();
	if (a != NULL)
	{
		a->tL = t;
		double dt = a->ta();
		if (dt < 0.0)
		{
			exception err("Negative time advance",a);
			throw err;
		}
		if (dt == DBL_MAX)
			sched.schedule(a,DBL_MAX);
		else
			sched.schedule(a,t+dt);
	}
	else
	{
		Set<Devs<X>*> components;
		model->typeIsNetwork()->getComponents(components);
		typename Set<Devs<X>*>::iterator iter = components.begin();
		for (; iter != components.end(); iter++)
		{
			schedule(*iter,t);
		}
	}
}

template <class X>
void Simulator<X>::inject_event(Atomic<X>* model, X& value)
{
	if (model->active == false)
	{
		model->active = true;
		activated.insert(model);
	}
	if (model->x == NULL)
	{
		model->x = io_pool.make_obj();
	}
	model->x->insert(value);
}

template <class X>
void Simulator<X>::route(Network<X>* parent, Devs<X>* src, X& x)
{
	// Notify event listeners if this is an output event
	if (parent != src)
		notify_output_listeners(src,x,sched.minPriority());
	// No one to do the routing, so return
	if (parent == NULL) return;
	// Compute the set of receivers for this value
	Bag<Event<X> >* recvs = recv_pool.make_obj();
	parent->route(x,src,*recvs);
	// Deliver the event to each of its targets
	Atomic<X>* amodel = NULL;
	typename Bag<Event<X> >::iterator recv_iter = recvs->begin();
	for (; recv_iter != recvs->end(); recv_iter++)
	{
		// Check for self-influencing error condition
		if (src == (*recv_iter).model)
		{
			exception err("Model tried to influence self",src);
			throw err;
		}
		/**
		if the destination is an atomic model, add the event to the IO bag for that model
		and add model to the list of activated models
		*/
		amodel = (*recv_iter).model->typeIsAtomic();
		if (amodel != NULL)
		{
			inject_event(amodel,(*recv_iter).value);
		}
		// if this is an external output from the parent model
		else if ((*recv_iter).model == parent)
		{
			route(parent->getParent(),parent,(*recv_iter).value);
		}
		// otherwise it is an input to a coupled model
		else
		{
			route((*recv_iter).model->typeIsNetwork(),
			(*recv_iter).model,(*recv_iter).value);
		}
	}
	recvs->clear();
	recv_pool.destroy_obj(recvs);
}

template <class X>
void Simulator<X>::exec_event(Atomic<X>* model, bool internal, double t)
{
	// Compute the state change
	if (model->x == NULL)
	{
		model->delta_int();
	}
	else if (internal)
	{
		model->delta_conf(*(model->x));
	}
	else
	{
		model->delta_ext(t-model->tL.t,*(model->x));
	}
	// Notify any listeners
	notify_state_listeners(model,t);
	// Check for a model transition
	if (model->model_transition() && model->getParent() != NULL)
	{
		model_func_eval_set.insert(model->getParent());
	}
}

template <class X>
void Simulator<X>::getAllChildren(Network<X>* model, Set<Devs<X>*>& s)
{
	Set<Devs<X>*> tmp;
	// Get the component set
	model->getComponents(tmp);
	// Find the components of type network and update s recursively
	typename Set<Devs<X>*>::iterator iter;
	for (iter = tmp.begin(); iter != tmp.end(); iter++)
	{
		if ((*iter)->typeIsNetwork() != NULL)
		{
			getAllChildren((*iter)->typeIsNetwork(),s);
		}
	}
	// Add all of the local level elements to s
	for (iter = tmp.begin(); iter != tmp.end(); iter++)
	{
		s.insert(*iter);
	}
}

template <class X>
Simulator<X>::~Simulator()
{
	// Clean up the models with stale IO
	typename Bag<Atomic<X>*>::iterator imm_iter;
	for (imm_iter = imm.begin(); imm_iter != imm.end(); imm_iter++)
	{
		clean_up(*imm_iter);
	}
	for (imm_iter = activated.begin(); imm_iter != activated.end(); imm_iter++)
	{
		clean_up(*imm_iter);
	}
}

} // End of namespace

#endif
