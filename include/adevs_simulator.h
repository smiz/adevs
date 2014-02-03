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
#ifndef __adevs_simulator_h_
#define __adevs_simulator_h_
#include "adevs_abstract_simulator.h"
#include "adevs_models.h"
#include "adevs_event_listener.h"
#include "adevs_sched.h"
#include "adevs_bag.h"
#include "adevs_set.h"
#include "object_pool.h"
#include "adevs_lp.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace adevs
{

/**
 * This Simulator class implements the DEVS simulation algorithm.
 * Its methods throw adevs::exception objects if any of the DEVS model
 * constraints are violated (i.e., a negative time advance or a model
 * attempting to send an input directly to itself).
 */
template <class X, class T = double> class Simulator:
	public AbstractSimulator<X,T>,
	private Schedule<X,T>::ImminentVisitor
{
	public:
		/**
		 * Create a simulator for a model. The simulator
		 * constructor will fail and throw an adevs::exception if the
		 * time advance of any component atomic model is less than zero.
		 * @param model The model to simulate
		 */
		Simulator(Devs<X,T>* model):
			AbstractSimulator<X,T>(),
			Schedule<X,T>::ImminentVisitor(),
			lps(NULL)
		{
			schedule(model,adevs_zero<T>());
		}
		/**
		 * Get the model's next event time
		 * @return The absolute time of the next event
		 */
		T nextEventTime()
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
		void execUntil(T tend)
		{
			while (nextEventTime() <= tend 
                   && nextEventTime() < adevs_inf<T>()) {
				execNextEvent();
            }
		}
		/**
		 * Compute the output values of the imminent component models 
		 * if these values have not already been computed.  This will
		 * notify registered EventListeners as the outputs are produced. 
		 */
		void computeNextOutput();
		/**
		 * Apply the bag of inputs at time t and then compute the next model
		 * states. Requires that lastEventTime() <= t <= nextEventTime().
		 * This, in effect, implements the state transition function of 
		 * the resultant model.
		 * @param input A bag of (input target,value) pairs
		 * @param t The time at which the input takes effect
		 */
		void computeNextState(Bag<Event<X,T> >& input, T t);
		/**
		 * Deletes the simulator, but leaves the model intact. The model must
		 * exist when the simulator is deleted.  Delete the model only after
		 * the Simulator is deleted.
		 */
		~Simulator();
		/**
		 * Assign a model to the simulator. This has the same effect as passing
		 * the model to the constructor.
		 */
		void addModel(Atomic<X,T>* model) 
		{
			schedule(model,adevs_zero<T>());
		}
		/**
		 * Create a simulator that will be used by an LP as part of a parallel
		 * simulation. This method is used by the parallel simulator.
		 */
		Simulator(LogicalProcess<X,T>* lp);
		/**
		 * <P>Call this method to indicate that all subsequent calls are part
		 * of a lookahead calculation. Lookahead calculations will cause
		 * the atomic models involved to save their states at the point
		 * that the lookahead calculation was begun. These states are
		 * restored when the lookahead calculation ends.</P>
		 * <P>Lookahead calculations are done with the lookNextEvent method,
		 * which may throw a lookahead_impossible_exception. This occurs when
		 * the simulator calculate a new state for an atomic model
		 * whose beginLookahead method is unsupported.</P>
		 */
		void beginLookahead();
		/**
		 * This call terminates a lookahead calculation and restores the
		 * models to their states when beginLookahead was called.
		 */
		void endLookahead();
		/**
		 * Look at future events assuming a input trajector with only
		 * non-events. This has the same effect as calling execNextEvent
		 * but the simulator can be restored to its prior state by
		 * calling endLookahead. 
		 */
		void lookNextEvent();
	private:
		typedef enum { OUTPUT_OK, OUTPUT_NOT_OK, RESTORING_OUTPUT } OutputStatus;
		// Structure to support parallel computing by a logical process
		struct lp_support
		{
			// The processor that this simulator works for
			LogicalProcess<X,T>* lp;
			bool look_ahead, stop_forced;
			OutputStatus out_flag;
			Bag<Atomic<X,T>*> to_restore;
		};
		// This is NULL if the simulator is not supporting a logical process
		lp_support* lps;
		// Bogus input bag for execNextEvent() method
		Bag<Event<X,T> > bogus_input;
		// The event schedule
		Schedule<X,T> sched;
		// List of models that are imminent or activated by input
		Bag<Atomic<X,T>*> activated;
		// Pools of preallocated, commonly used objects
		object_pool<Bag<X> > io_pool;
		object_pool<Bag<Event<X,T> > > recv_pool;
		// Sets for computing structure changes.
		Bag<Devs<X,T>*> added;
		Bag<Devs<X,T>*> removed;
		Set<Devs<X,T>*> next;
		Set<Devs<X,T>*> prev;
		// Model transition functions are evaluated from the bottom up!
		struct bottom_to_top_depth_compare
		{
			bool operator()(const Network<X,T>* m1, const Network<X,T>* m2) const
			{
				unsigned long int d1 = 0, d2 = 0;
				// Compute depth of m1
				const Network<X,T>* m = m1->getParent();
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
			bool operator()(const Devs<X,T>* m1, const Devs<X,T>* m2) const
			{
				unsigned long int d1 = 0, d2 = 0;
				// Compute depth of m1
				const Network<X,T>* m = m1->getParent();
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
		std::set<Network<X,T>*,bottom_to_top_depth_compare> model_func_eval_set;
		std::set<Devs<X,T>*,top_to_bottom_depth_compare> sorted_removed;
		/**
		 * Recursively add the model and its elements to the schedule 
		 * using t as the time of last event.
		 */
		void schedule(Devs<X,T>* model, T t);
		/// Route an event generated by the source model contained in the parent model.
		void route(Network<X,T>* parent, Devs<X,T>* src, X& x);
		/**	
		 * Add an input to the input bag of an an atomic model. If the 
		 * model is not already active , then this method adds the model to
		 * the activated bag.
		 */
		void inject_event(Atomic<X,T>* model, X& value);
		/**
		 * Recursively remove a model and its components from the schedule 
		 * and the imminent/activated bags
		 */
		void unschedule_model(Devs<X,T>* model);
		/**
		 * Delete any thing in the output bag, and return the input
		 * and output bags to the pools.
		 * Recursively clean up network model components.
		 */
		void clean_up(Devs<X,T>* model);
		/**
		 * Execute the state transition function using t to compute the
		 * elapsed time as t-model->tL. This adds the model to the nx bag 
		 * if it is a network executive and updates the added
		 * and removed sets. 
		 */
		void exec_event(Atomic<X,T>* model, T t);
		/**
		 * Construct the complete descendant set of a network model and store it in s.
		 */
		void getAllChildren(Network<X,T>* model, Set<Devs<X,T>*>& s);
		/**
		 * Update data structures needed for a reset of the simulator
		 * following a speculative lookahead. Returns true if the
		 * lookahead can be managed. False otherwise.
		 */
		bool manage_lookahead_data(Atomic<X,T>* model);
		/**
		 * Visit method inhereted from ImminentVisitor
		 */
		void visit(Atomic<X,T>* model);
};

template <class X, class T>
void Simulator<X,T>::visit(Atomic<X,T>* model)
{
	assert(model->y == NULL);
	model->y = io_pool.make_obj();
	// Put it in the active list if it is not already there
	if (model->x == NULL)
		activated.insert(model);
	// Compute output functions and route the events. The bags of output
	// are held for garbage collection at a later time.
	model->output_func(*(model->y));
	// Route each event in y
	for (typename Bag<X>::iterator y_iter = model->y->begin(); 
		y_iter != model->y->end(); y_iter++)
	{
		route(model->getParent(),model,*y_iter);
	}
}

template <class X, class T>
void Simulator<X,T>::computeNextOutput()
{
	// If the imminent set is up to date, then just return
	if (activated.empty() == false) return;
	// Get the imminent models from the schedule. 
	sched.visitImminent(this);
}

template <class X, class T>
void Simulator<X,T>::computeNextState(Bag<Event<X,T> >& input, T t)
{
	// Clean up if there was a previous IO calculation
	if (t < sched.minPriority())
	{
		typename Bag<Atomic<X,T>*>::iterator iter;
		for (iter = activated.begin(); iter != activated.end(); iter++)
		{
			clean_up(*iter);
		}
		activated.clear();
	}
	// Otherwise, if the internal IO needs to be computed, do it
	else if (t == sched.minPriority())
	{
		computeNextOutput();
	}
	// Apply the injected inputs
	for (typename Bag<Event<X,T> >::iterator iter = input.begin(); 
	iter != input.end(); iter++)
	{
		Atomic<X,T>* amodel = (*iter).model->typeIsAtomic();
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
	 * Compute the states of atomic models.  Store Network models that 
	 * need to have their model transition function evaluated in a
	 * special container that will be used when the structure changes are
	 * computed (see exec_event(.)).
	 */
	for (typename Bag<Atomic<X,T>*>::iterator iter = activated.begin(); 
		iter != activated.end(); iter++)
	{
		exec_event(*iter,t); 
	}
	/**
	 * Compute model transitions and build up the prev (pre-transition)
	 * and next (post-transition) component sets. These sets are built
	 * up from only the models that have the model_transition function
	 * evaluated.
	 */
	if (model_func_eval_set.empty() == false)
	{
		while (!model_func_eval_set.empty())
		{
			Network<X,T>* network_model = *(model_func_eval_set.begin());
			model_func_eval_set.erase(model_func_eval_set.begin());
			getAllChildren(network_model,prev);
			if (network_model->model_transition() &&
					network_model->getParent() != NULL)
			{
				model_func_eval_set.insert(network_model->getParent());
			}
			getAllChildren(network_model,next);
		}
		// Find the set of models that were added.
		set_assign_diff(added,next,prev);
		// Find the set of models that were removed
		set_assign_diff(removed,prev,next);
		// Intersection of added and removed is always empty, so no need to look
		// for models in both (an earlier version of the code did this).
		next.clear();
		prev.clear();
		/** 
		 * The model adds are processed first.  This is done so that, if any
		 * of the added models are components something that was removed at
		 * a higher level, then the models will not have been deleted when
		 * trying to schedule them.
		 */
		for (typename Bag<Devs<X,T>*>::iterator iter = added.begin(); 
			iter != added.end(); iter++)
		{
			schedule(*iter,t);
		}
		// Done with the additions
		added.clear();
		// Remove the models that are in the removed set.
		for (typename Bag<Devs<X,T>*>::iterator iter = removed.begin(); 
			iter != removed.end(); iter++)
		{
			clean_up(*iter);
			unschedule_model(*iter);
			// Add to a sorted remove set for deletion
			sorted_removed.insert(*iter); 
		}
		// Done with the unsorted remove set
		removed.clear();
		// Delete the sorted removed models
		while (!sorted_removed.empty())
		{
			// Get the model to erase
			Devs<X,T>* model_to_remove = *(sorted_removed.begin());
			// Remove the model
			sorted_removed.erase(sorted_removed.begin());
			/**
			 * If this model has children, then remove them from the 
			 * deletion set. This will avoid double delete problems.
			 */
			if (model_to_remove->typeIsNetwork() != NULL)
			{
				getAllChildren(model_to_remove->typeIsNetwork(),prev);
				typename Set<Devs<X,T>*>::iterator iter = prev.begin();
				for (; iter != prev.end(); iter++)
					sorted_removed.erase(*iter);
				prev.clear();
			}
			// Delete the model and its children
			delete model_to_remove;
		}
		// Removed sets should be empty now
		assert(prev.empty());
		assert(sorted_removed.empty());
	} // End of the structure change
	// Cleanup and reschedule models that changed state in this iteration
	// and survived the structure change phase.
	for (typename Bag<Atomic<X,T>*>::iterator iter = activated.begin(); 
		iter != activated.end(); iter++)
	{
		clean_up(*iter);
		schedule(*iter,t);
	}
	// Empty the bags
	activated.clear();
	// If we are looking ahead, throw an exception if a stop was forced
	if (lps != NULL && lps->stop_forced)
	{
		lookahead_impossible_exception err;
		throw err;
	}
}

template <class X, class T>
void Simulator<X,T>::clean_up(Devs<X,T>* model)
{
	Atomic<X,T>* amodel = model->typeIsAtomic();
	if (amodel != NULL)
	{
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
		Set<Devs<X,T>*> components;
		model->typeIsNetwork()->getComponents(components);
		for (typename Set<Devs<X,T>*>::iterator iter = components.begin();
		iter != components.end(); iter++)
		{
			clean_up(*iter);
		}
	}
}

template <class X, class T>
void Simulator<X,T>::unschedule_model(Devs<X,T>* model)
{
	if (model->typeIsAtomic() != NULL)
	{
		sched.schedule(model->typeIsAtomic(),adevs_inf<T>());
		activated.erase(model->typeIsAtomic());
	}
	else
	{
		Set<Devs<X,T>*> components;
		model->typeIsNetwork()->getComponents(components);
		for (typename Set<Devs<X,T>*>::iterator iter = components.begin();
		iter != components.end(); iter++)
		{
			unschedule_model(*iter);
		}
	}
}

template <class X, class T>
void Simulator<X,T>::schedule(Devs<X,T>* model, T t)
{
	Atomic<X,T>* a = model->typeIsAtomic();
	if (a != NULL)
	{
		a->tL = t;
		T dt = a->ta();
		if (dt < adevs_zero<T>())
		{
			exception err("Negative time advance",a);
			throw err;
		}
		if (dt == adevs_inf<T>())
			sched.schedule(a,adevs_inf<T>());
		else
			sched.schedule(a,t+dt);
	}
	else
	{
		Set<Devs<X,T>*> components;
		model->typeIsNetwork()->getComponents(components);
		typename Set<Devs<X,T>*>::iterator iter = components.begin();
		for (; iter != components.end(); iter++)
		{
			schedule(*iter,t);
		}
	}
}

template <class X, class T>
void Simulator<X,T>::inject_event(Atomic<X,T>* model, X& value)
{
	if (model->x == NULL)
	{
		if (model->y == NULL)
			activated.insert(model);
		model->x = io_pool.make_obj();
	}
	model->x->insert(value);
}

template <class X, class T>
void Simulator<X,T>::route(Network<X,T>* parent, Devs<X,T>* src, X& x)
{
	// Notify event listeners if this is an output event
	if (parent != src && (lps == NULL || lps->out_flag != RESTORING_OUTPUT))
		this->notify_output_listeners(src,x,sched.minPriority());
	// No one to do the routing, so return
	if (parent == NULL) return;
	// Compute the set of receivers for this value
	Bag<Event<X,T> >* recvs = recv_pool.make_obj();
	parent->route(x,src,*recvs);
	// Deliver the event to each of its targets
	Atomic<X,T>* amodel = NULL;
	typename Bag<Event<X,T> >::iterator recv_iter = recvs->begin();
	for (; recv_iter != recvs->end(); recv_iter++)
	{
		// Check for self-influencing error condition
		if (src == (*recv_iter).model)
		{
			exception err("Model tried to influence self",src);
			throw err;
		}
		/**
		 * If the destination is an atomic model, add the event to the IO bag
		 * for that model and add model to the list of activated models
		 */
		amodel = (*recv_iter).model->typeIsAtomic();
		if (amodel != NULL)
		{
			// Inject it only if it is assigned to our processor
			if (lps == NULL || amodel->getProc() == lps->lp->getID())
				inject_event(amodel,(*recv_iter).value);
			// Otherwise tell the lp about it
			else if (lps->out_flag != RESTORING_OUTPUT)
				lps->lp->notifyInput(amodel,(*recv_iter).value);
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

template <class X, class T>
void Simulator<X,T>::exec_event(Atomic<X,T>* model, T t)
{
	if (!manage_lookahead_data(model)) return;
	// Internal event
	if (model->x == NULL)
		model->delta_int();
	// Confluent event
	else if (model->y != NULL)
		model->delta_conf(*(model->x));
	// External event
	else
		model->delta_ext(t-model->tL,*(model->x));
	// Notify any listeners
	this->notify_state_listeners(model,t);
	// Check for a model transition
	if (model->model_transition() && model->getParent() != NULL)
	{
		model_func_eval_set.insert(model->getParent());
	}
}

template <class X, class T>
void Simulator<X,T>::getAllChildren(Network<X,T>* model, Set<Devs<X,T>*>& s)
{
	Set<Devs<X,T>*> tmp;
	// Get the component set
	model->getComponents(tmp);
	// Add all of the local level elements to s
	s.insert(tmp.begin(),tmp.end());
	// Find the components of type network and update s recursively
	typename Set<Devs<X,T>*>::iterator iter;
	for (iter = tmp.begin(); iter != tmp.end(); iter++)
	{
		if ((*iter)->typeIsNetwork() != NULL)
		{
			getAllChildren((*iter)->typeIsNetwork(),s);
		}
	}
}

template <class X, class T>
Simulator<X,T>::~Simulator()
{
	// Clean up the models with stale IO
	typename Bag<Atomic<X,T>*>::iterator iter;
	for (iter = activated.begin(); iter != activated.end(); iter++)
	{
		clean_up(*iter);
	}
}

template <class X, class T>
Simulator<X,T>::Simulator(LogicalProcess<X,T>* lp):
	AbstractSimulator<X,T>()
{
	lps = new lp_support;
	lps->lp = lp;
	lps->look_ahead = false;
	lps->stop_forced = false;
	lps->out_flag = OUTPUT_OK;
}

template <class X, class T>
void Simulator<X,T>::beginLookahead()
{
	if (lps == NULL)
	{
		adevs::exception err("tried to lookahead without lp support");
		throw err;
	}
	lps->look_ahead = true;
	if (!activated.empty())
		lps->out_flag = OUTPUT_NOT_OK; 
}

template <class X, class T>
void Simulator<X,T>::lookNextEvent()
{
	execNextEvent();
}

template <class X, class T>
void Simulator<X,T>::endLookahead()
{
	if (lps == NULL) return;
	typename Bag<Atomic<X,T>*>::iterator iter = lps->to_restore.begin();
	for (; iter != lps->to_restore.end(); iter++)
	{
		(*iter)->endLookahead();
		schedule(*iter,(*iter)->tL_cp);
		(*iter)->tL_cp = adevs_sentinel<T>();
		assert((*iter)->x == NULL);
		assert((*iter)->y == NULL);
	}
	lps->to_restore.clear();
	assert(activated.empty());
	if (lps->out_flag == OUTPUT_NOT_OK)
	{
		lps->out_flag = RESTORING_OUTPUT;
		computeNextOutput();
		lps->out_flag = OUTPUT_OK;
	}
	lps->look_ahead = false;
	lps->stop_forced = false;
}

template <class X, class T>
bool Simulator<X,T>::manage_lookahead_data(Atomic<X,T>* model)
{
	if (lps == NULL) return true;
	if (lps->look_ahead && model->tL_cp < adevs_zero<T>())
	{
		lps->to_restore.insert(model);
		model->tL_cp = model->tL;
		try
		{
			model->beginLookahead();
		}
		catch(method_not_supported_exception err)
		{
			lps->stop_forced = true;
		}
	}
	return !(lps->stop_forced);
}

} // End of namespace

#endif
