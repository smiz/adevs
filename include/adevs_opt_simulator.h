#ifndef __adevs_opt_simulator_h_
#define __adevs_opt_simulator_h_
#include "adevs.h"
#include "adevs_lp.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <omp.h>
#include "adevs_sched.h"

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
		an event in every iteration of the optimistic simulator.  
		*/
		OptSimulator(Devs<X>* model, int max_batch_size = 1000, bool thread_safe_listeners = false);
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
			if (sched.empty()) return Time::Inf();
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
		/// Are event listeners thread safe?
		const bool thrd_safe_listeners;
		omp_lock_t listener_lock;
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
OptSimulator<X>::OptSimulator(Devs<X>* model, int max_batch_size, bool thread_safe_listeners):
	top_model(model),
	max_batch_size(max_batch_size),
	thrd_safe_listeners(thrd_safe_listeners)
{
	omp_init_lock(&listener_lock);
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
				if (!thrd_safe_listeners) omp_set_lock(&listener_lock);
				for (liter = listeners.begin(); liter != listeners.end(); liter++)
					(*liter)->outputEvent(event,(*oiter).t.t);
				if (!thrd_safe_listeners) omp_unset_lock(&listener_lock);
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
			// Otherwise report every uncommitted state
			else if ((*siter).t > a->lp->getLastCommit())
			{
				a->lp->setLastCommit((*siter).t);
				if (!thrd_safe_listeners) omp_set_lock(&listener_lock);
				for (liter = listeners.begin(); liter != listeners.end(); liter++)
					(*liter)->stateChange(a,(*siter).t.t,(*siter).data);
				if (!thrd_safe_listeners) omp_unset_lock(&listener_lock);
			}
		}
		// Report the last state if it is prior to gvt
		if (a->lp->getLocalStateTime() < effective_gvt &&
				a->lp->getLocalStateTime() > a->lp->getLastCommit())
		{
			void* tmp_state = a->save_state();
			if (!thrd_safe_listeners) omp_set_lock(&listener_lock);
			for (liter = listeners.begin(); liter != listeners.end(); liter++)
				(*liter)->stateChange(a,a->lp->getLocalStateTime().t,tmp_state);
			if (!thrd_safe_listeners) omp_unset_lock(&listener_lock);
			a->gc_state(tmp_state);
			a->lp->setLastCommit(a->lp->getLocalStateTime());
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
	// Keep track of the global virtual time
	Time actual_gvt = totalNextEventTime();
	// Run until global virtual time meets or exceeds gvt
	while (actual_gvt <= stop_time && actual_gvt < DBL_MAX)
	{
		// prepare the list of ready models
		int batch_size = sched.getSize();
		if (batch_size > max_batch_size) batch_size = max_batch_size;
		int i = 0;	
		for (i = 0; i < batch_size; i++)
		{
			batch[i] = sched.get(i+1);
			batch[i]->lp->setActive(true);
		}
		// Clean up a bit if we can, then process the input lists 
		#pragma omp parallel for default(shared) private(i)
		for (i = 0; i < batch_size; i++)
		{
			fossil_collect_and_commit(batch[i],actual_gvt);
			batch[i]->lp->processInput();	
		}
		// Speculative execution of state transition and output functions
		#pragma omp parallel for default(shared) private(i)
		for (i = 0; i < batch_size; i++)
		{						
			batch[i]->lp->execEvents(10);
		}		
		// Reschedule the models in the batch and reset their active flags
		for (i = 0; i < batch_size; i++)
		{
			sched.schedule(batch[i],batch[i]->lp->getNextEventTime());
			batch[i]->lp->setActive(false);
		}
		// Schedule the activated models
		while (!active_list.empty())
		{
			sched.schedule(active_list.back()->getModel(),active_list.back()->getNextEventTime());
			active_list.back()->setActive(false);
			active_list.pop_back();
		}
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
	omp_destroy_lock(&listener_lock);
}

} // End of namespace

#endif
