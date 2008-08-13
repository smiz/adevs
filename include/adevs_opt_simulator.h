/***************
Copyright (C) 2008 by James Nutaro

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
#ifndef __adevs_opt_simulator_h_
#define __adevs_opt_simulator_h_
#include "adevs.h"
#include "adevs_lp.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace adevs
{

/**
 * This class implements an optimistic simulation algorithm that uses
 * the OpenMP standard for its threading functionality. Your model
 * must satisfy four properties for this simulator to work properly:
 * (1) Every Atomic model must implement the methods for saving and restoring
 * its state, (2) Atomic models can not share any state variables (read
 * or write), (3) the route methods of all of the Network models must
 * be re-entrant, and (4) there are no structure changes. 
*/
template <class X> class OptSimulator 
{
	public:
		/**
		Create a simulator for the provided model. The simulator
		constructor will fail and throw an adevs::exception if the
		time advance of any component atomic model is less than zero.
		The batch size parameter controls the potential degree of parallelism
		and parallel overhead; it is the number of models that will process
		an event in every iteration of the optimistic simulator. The 
		fc_iterations flag is the maximum number of iterations between
		garbage collection events; it determines the serial portion of
		the simulation and the memory overhead. 
		*/
		OptSimulator(Devs<X>* model, int max_batch_size = 500,
				int fc_iterations = INT_MAX);
		/**
		Add an event listener that will be notified of output events 
		produced by the model.
		*/
		void addEventListener(EventListener<X>* l)
		{
			listeners.insert(l);
		}
		/// Remove an event listener
		void removeEventListener(EventListener<X>* l)
		{
			listeners.erase(l);
		}
		/// Get the model's next event time
		double nextEventTime()
		{
			if (sched.empty()) return DBL_MAX;
			return sched.minPriority().t;
		}
		/// Get the complete next event time
		Time totalNextEventTime()
		{
			if (sched.empty()) return Time(DBL_MAX);
			else return sched.minPriority();
		}
		/**
		 * Execute the simulation until the next event time is greater
		 * than the specified value.
		 */
		void execUntil(double gvt)
		{
			Time t_stop(gvt,UINT_MAX);
			if (gvt == DBL_MAX) t_stop.c = 0;
			execUntil(t_stop);
		}
		/**
		 * Execute the simulator until the next event time is greater
		 * than the specified value.
		 */
		void execUntil(Time gvt);
		/**
		Deletes the simulator, but leaves the model intact. The model must
		exist when the simulator is deleted.  Delete the model only after
		the simulator is deleted.
		*/
		~OptSimulator();
	private:
		/// Top of the model tree
		Devs<X>* top_model;
		/// Eternal event listeners
		Bag<EventListener<X>*> listeners;
		/// The event schedule
		Schedule<X,Time> sched;
		/// List of imminent models
		std::vector<LogicalProcess<X>*> active_list;
		/// List of models with events to execute in the current round
		Atomic<X>** batch;
		/// Number of models in the list
		const int max_batch_size;
		/// Number of simulation iterations between fossil collection rounds
		const int fc_iterations;
		/**
		 * Recursively initialize the model by assigning an lp to each atomic
		 * model and putting active models into the schedule.
		*/
		void initialize(Devs<X>* model);
		/**
		 * Recursively delete all of the logical process objects.
		 */
		void cleanup(Devs<X>* model);
		/**
		 * Recursively do fossil collection and commit events. Return the largest timestamp
		 * of the committed events.
		 */
		void fossil_collect_and_commit(Devs<X>* model, Time effective_gvt);
};

template <class X>
OptSimulator<X>::OptSimulator(Devs<X>* model, int max_batch_size, 
		int fc_iterations):
	top_model(model),
	max_batch_size(max_batch_size),
	fc_iterations(fc_iterations)
{
	batch = new Atomic<X>*[max_batch_size];
	initialize(model);
}

template <class X>
void OptSimulator<X>::initialize(Devs<X>* model)
{
	Atomic<X>* a = model->typeIsAtomic();
	if (a != NULL)
	{
		a->lp = new LogicalProcess<X>(a,&active_list);
		sched.schedule(a,a->lp->getNextEventTime());
	}
	else
	{
		Set<Devs<X>*> components;
		model->typeIsNetwork()->getComponents(components);
		typename Set<Devs<X>*>::iterator iter = components.begin();
		for (; iter != components.end(); iter++)
		{
			initialize(*iter);
		}
	}
}

template <class X>
void OptSimulator<X>::fossil_collect_and_commit(Devs<X>* model, Time effective_gvt)
{
	static Time ZERO(0.0,0);
	// Process an atomic model
	Atomic<X>* a = model->typeIsAtomic();
	if (a != NULL)
	{
		// Iterator over the listener list
		typename Bag<EventListener<X>*>::iterator liter;
		// Notify listeners of commited output events 
		typename std::list<Message<X> >::const_iterator oiter =
			a->lp->getOutput()->begin();
		for (; oiter != a->lp->getOutput()->end(); oiter++)
		{
			// Output is OK up to GVT
			if ((*oiter).t < effective_gvt)
			{
				Event<X> event(a,(*oiter).value);
				for (liter = listeners.begin(); liter != listeners.end(); liter++)
					(*liter)->outputEvent(event,(*oiter).t.t);
			}
			else
				break;
		}
		// Notify listeners of commited states
		typename std::list<CheckPoint>::iterator siter =
			a->lp->getStates()->begin();
		for (; siter != a->lp->getStates()->end(); siter++)
		{
			// Stop when we reach gvt
			if ((*siter).t >= effective_gvt)
			{
				break;
			}
			// Otherwise report every save state except the initial one
			else if ((*siter).t > ZERO && (*siter).reported == false)
			{
				(*siter).reported = true;
				for (liter = listeners.begin(); liter != listeners.end(); liter++)
				{
					(*liter)->stateChange(a,(*siter).t.t,(*siter).data);
				}
			}
		}
		// Cleanup
		a->lp->fossilCollect(effective_gvt);
	}
	// Otherwise continue traversing the model tree
	else
	{
		Set<Devs<X>*> components;
		model->typeIsNetwork()->getComponents(components);
		typename Set<Devs<X>*>::iterator iter = components.begin();
		for (; iter != components.end(); iter++)
		{
			fossil_collect_and_commit(*iter,effective_gvt);
		}
	}
}

template <class X>
void OptSimulator<X>::cleanup(Devs<X>* model)
{
	Atomic<X>* a = model->typeIsAtomic();
	if (a != NULL)
	{
		delete a->lp;
		a->lp = NULL;
	}
	else
	{
		Set<Devs<X>*> components;
		model->typeIsNetwork()->getComponents(components);
		typename Set<Devs<X>*>::iterator iter = components.begin();
		for (; iter != components.end(); iter++)
		{
			cleanup(*iter);
		}
	}
}

template <class X>
void OptSimulator<X>::execUntil(Time stop_time)
{
	// Exception for handling errors inside of the parallel loop
	exception caught_exception("",NULL);
	bool err = false;
	// Iteration counter for garbage collection
	int iterations = 0;
	// Keep track of the global virtual time
	Time actual_gvt = totalNextEventTime();
	// Run until global virtual time meets or exceeds gvt
	while (actual_gvt <= stop_time && actual_gvt < DBL_MAX)
	{
		if (iterations == fc_iterations)
		{
			fossil_collect_and_commit(top_model,actual_gvt);
			iterations = 0;
		}
		iterations++;
		// Determine the batch size for this iteration
		int batch_size = max_batch_size;
		if (batch_size > (int)sched.getSize())
			batch_size = (int)sched.getSize();
		/**
		 * Get a batch of models and execute an event for each
		 * one. This loop tries to exploit parallelism in the model
		 * by speculatively executing events for models whose
		 * next event times are close to the global virtual time.
		 */
		#pragma omp parallel for
		for (int i = 0; i < batch_size; i++)
		{
			/*
			 * Note that a model could end up in the batch set AND in
			 * the activated list. This will result in two attempts
			 * to schedule the model, but that is ok because the second
			 * attempt will just leave the model in place.
			 */
			batch[i] = sched.get(i+1);
			batch[i]->lp->setActive(true);
			try
			{
				batch[i]->lp->execNextEvent();
			}
			catch(exception except)
			{
				err = true;
				caught_exception = except;
			}
		}
		// Throw the last exception that was caught
		if (err) throw caught_exception;
		// Reschedule the models in the batch and reset their active flags
		for (int i = 0; i < batch_size; i++)
		{
			sched.schedule(batch[i],batch[i]->lp->getNextEventTime());
			batch[i]->lp->setActive(false);
		}
		// Schedule the activated models
		typename std::vector<LogicalProcess<X>*>::iterator iter =
			active_list.begin();
		for (; iter != active_list.end(); iter++)
		{
			sched.schedule((*iter)->getModel(),(*iter)->getNextEventTime());
			(*iter)->setActive(false);
		}
		active_list.clear();
		// Get the global virtual time 
		actual_gvt = totalNextEventTime();
	}
	// Do fossil collection and send event notifications
	if (actual_gvt > stop_time) actual_gvt = stop_time;
	fossil_collect_and_commit(top_model,actual_gvt);
}

template <class X>
OptSimulator<X>::~OptSimulator()
{
	cleanup(top_model);
	delete [] batch;
}

} // End of namespace

#endif
