#ifndef __adevs_opt_simulator_h_
#define __adevs_opt_simulator_h_
#include "adevs.h"
#include "adevs_abstract_simulator.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <omp.h>
#include "adevs_sched.h"
#include "adevs_list.h"

namespace adevs
{

template <typename X> class LogicalProcess
{
	public:
		LogicalProcess()
		{
			checkpoint = NULL;
			targets = outputs = NULL;
			needs_update = false;
			is_safe = true;
		}
		bool needs_update;
		bool is_safe;
		void* checkpoint;
		Atomic<X>* model;
		Bag<Event<X> > *targets, *outputs; 
		int wait_for_safe()
		{
			int stalled = 0;
			bool* is_safe_ptr = &is_safe;
			while (!(*is_safe_ptr))
			{
				stalled = 1;
				#pragma omp flush(is_safe_ptr)
			}
			if (!needs_update)
			{
				#pragma omp flush
			} 
			return stalled;
		}
};

/**
 * This class implements an optimistic simulation algorithm that uses
 * the OpenMP standard for its threading functionality. Your model
 * must satisfy four properties for this simulator to work properly:
 * (1) Every Atomic model must implement the methods for saving and restoring
 * its state, (2) Atomic models can not share any state variables (read
 * or write), (3) the route methods of all of the Network models must
 * be re-entrant, and (4) there are no structure changes. 
*/
template <class X> class OptSimulator:
   public AbstractSimulator<X>	
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
		OptSimulator(Devs<X>* model);
		/// Get the model's next event time
		double nextEventTime() { return sched.minPriority().t; }
		/**
		 * Execute the simulator until the next event time is greater
		 * than the specified value.
		 */
		void execUntil(double stop_time);
		/**
		Deletes the simulator, but leaves the model intact. The model must
		exist when the simulator is deleted.  Delete the model only after
		the simulator is deleted.
		*/
		~OptSimulator();
		/// Get the number of early outputs
		unsigned getEarlyOutputCount() const { return early_output; }
		/// Get the number of stalls while waiting for speculation
		unsigned getStallCount() const { return int_stalls+ext_stalls; }
		/// Get the number of stalls on external events
		unsigned getExtStallCount() const { return ext_stalls; }
		/// Get the number of stalls on internal events
		unsigned getIntStallCount() const { return int_stalls; }
		/// Get the number of outputs computed just in time
		unsigned getInTimeOutputCount() const { return jit_output; }
	private:
		/// The event schedule
		Schedule<X,Time> sched;
		/// List of imminent models
		Bag<Atomic<X>*> imm;
		/// List of models activated by input
		Bag<Atomic<X>*> activated;
		bool halt;
		unsigned early_output, jit_output, ext_stalls, int_stalls;
		LogicalProcess<X>** pending;
		/// Pools of preallocated, commonly used objects
		object_pool<Bag<X> > io_pool;
		object_pool<Bag<Event<X> > >* recv_pool;
		const int SAFE_THREAD;
		const int thread_count;
		void inject_event(Atomic<X>* model, X& value);	
		void exec_event(Atomic<X>* model, bool internal, Time t);
		void schedule(Atomic<X>* model, Time t);
		void init(Devs<X>* model);
		void route(Network<X>* parent, Devs<X>* src, X& x, int thrd_id,
				Bag<Event<X> >* targets = NULL, Bag<Event<X> >* outputs = NULL);
		void speculate(int thread_num);
		void simSafe(double tend);
		void prep_for_speculation(Atomic<X>* model);
		void clean_up_output(Atomic<X>* model);
}; 

template <class X>
OptSimulator<X>::OptSimulator(Devs<X>* model):
	AbstractSimulator<X>(),
	SAFE_THREAD(0),
	thread_count(omp_get_max_threads())
{
	recv_pool = new object_pool<Bag<Event<X> > >[thread_count];
	pending = new LogicalProcess<X>*[thread_count];
	for (int i = 0; i < thread_count; i++) pending[i] = NULL;
	jit_output = ext_stalls = int_stalls = early_output = 0;
	init(model);
}

template <class X>
OptSimulator<X>::~OptSimulator<X>()
{
}

template <class X>
void OptSimulator<X>::execUntil(double stop_time)
{
	halt = false;
	#pragma omp parallel
	{
		int thread_num = omp_get_thread_num();
		if (thread_num == SAFE_THREAD) simSafe(stop_time);
		else speculate(thread_num);
	}
}

template <class X>
void OptSimulator<X>::speculate(int thread_num)
{
	LogicalProcess<X>** lp_ptr = &(pending[thread_num]);
	while (true)
	{
		while (!halt && *lp_ptr == NULL)
		{
			#pragma omp flush(halt,lp_ptr)
		}
		if (halt) return;
		#pragma omp flush
		LogicalProcess<X>* lp = *lp_ptr;
		if (!lp->model->y->empty())
		{
			lp->model->gc_output(*(lp->model->y));
			lp->model->y->clear();
		}
		lp->outputs->clear();
		lp->targets->clear();
		// Speculate on a value
		lp->model->output_func(*(lp->model->y));
		// Route each event in y
		for (typename Bag<X>::iterator y_iter = lp->model->y->begin(); 
				y_iter != lp->model->y->end(); y_iter++)
		{
			route(lp->model->getParent(),lp->model,*y_iter,thread_num,lp->targets,lp->outputs);
		}
		// Speculate on the state
		if ((lp->checkpoint = lp->model->save_state()) != NULL)
		{
			lp->model->delta_int();
		}
		#pragma omp flush
		bool* safe_ptr = &(lp->is_safe);
		*safe_ptr = true;
		*lp_ptr = NULL;
		#pragma omp flush(lp_ptr,safe_ptr)
	}
}
	
template <class X>
void OptSimulator<X>::simSafe(double tstop)
{
	Time t;
	while (true)
	{
		// Find the imminent models and the next event time
		t = sched.minPriority();
		if (tstop < t.t || t.t == DBL_MAX)
		{
			halt = true;
			#pragma omp flush(halt)
			return;
		}
		sched.getImminent(imm);
		// Compute output and route events
		for (typename Bag<Atomic<X>*>::iterator imm_iter = imm.begin(); 
			imm_iter != imm.end(); imm_iter++)
		{
			Atomic<X>* model = *imm_iter;
			// If it has not had an output computed yet
			if (model->lp->needs_update)
			{
				model->y = io_pool.make_obj();
				jit_output++;
				// Compute its output
				model->output_func(*(model->y));
				// Route each event in y
				for (typename Bag<X>::iterator y_iter = model->y->begin(); 
					y_iter != model->y->end(); y_iter++)
				{
					route(model->getParent(),model,*y_iter,SAFE_THREAD);
				}
			}
			// Otherwise just copy the precomputed values when the lock
			// is released
			else
			{
				early_output++;
				// Make sure there are no outstanding parallel updates
				int_stalls += model->lp->wait_for_safe();
				// Copy the output values to their destinations
				for (typename Bag<Event<X> >::iterator y_iter = model->lp->targets->begin();
						y_iter != model->lp->targets->end(); y_iter++)
				{
					inject_event((*y_iter).model->typeIsAtomic(),(*y_iter).value);
				}
				for (typename Bag<Event<X> >::iterator y_iter = model->lp->outputs->begin();
						y_iter != model->lp->outputs->end(); y_iter++)
				{
					notify_output_listeners((*y_iter).model,(*y_iter).value,sched.minPriority().t);
				}
			}
		}
		for (typename Bag<Atomic<X>*>::iterator iter = imm.begin(); 
			iter != imm.end(); iter++)
		{
			assert((*iter)->lp->wait_for_safe() == 0);
			exec_event(*iter,true,t); // Internal and confluent transitions
		}
		for (typename Bag<Atomic<X>*>::iterator iter = activated.begin(); 
			iter != activated.end(); iter++)
		{
			ext_stalls += (*iter)->lp->wait_for_safe();
			exec_event(*iter,false,t); // External transitions
			schedule(*iter,t); 
		}
		// Clean up and reschedule the imminent models
		for (typename Bag<Atomic<X>*>::iterator iter = imm.begin(); 
			iter != imm.end(); iter++)
		{
			schedule(*iter,t);
		}
		imm.clear();
		activated.clear();
	}
}

template <class X>
void OptSimulator<X>::init(Devs<X>* model)
{
	Atomic<X>* a = model->typeIsAtomic();
	if (a != NULL)
	{
		a->lp = new LogicalProcess<X>();
		a->lp->model = a;
		schedule(a,Time(0.0,0));
	}
	else
	{
		Set<Devs<X>*> components;
		model->typeIsNetwork()->getComponents(components);
		typename Set<Devs<X>*>::iterator iter = components.begin();
		for (; iter != components.end(); iter++)
		{
			init(*iter);
		}
	}
}

template <class X>
void OptSimulator<X>::prep_for_speculation(Atomic<X>* model)
{
	if (model->y == NULL) model->y = io_pool.make_obj();
	if (model->lp->outputs == NULL)
	{
		model->lp->outputs = recv_pool[SAFE_THREAD].make_obj();
		model->lp->targets = recv_pool[SAFE_THREAD].make_obj();
	}
}

template <class X>
void OptSimulator<X>::clean_up_output(Atomic<X>* model)
{
	if (model->y != NULL)
	{
		if (!model->y->empty())
		{
			model->gc_output(*(model->y));
			model->y->clear();
		}
		io_pool.destroy_obj(model->y);
		model->y = NULL;
	}
	if (model->lp->outputs != NULL)
	{
		model->lp->outputs->clear();
		model->lp->targets->clear();
		recv_pool[SAFE_THREAD].destroy_obj(model->lp->targets);
		recv_pool[SAFE_THREAD].destroy_obj(model->lp->outputs);
		model->lp->outputs = model->lp->targets = NULL;
	} 
}

template <class X>
void OptSimulator<X>::schedule(Atomic<X>* model, Time t)
{
	// Compute the time of the next event
	double dt = model->ta();
	if (dt < 0.0)
	{
		exception err("Negative time advance",model);
		throw err;
	}
	// If there is no event, then clean its output objects and remove
	// it from the schedule
	if (dt == DBL_MAX)
	{
		// Clean up its output
		clean_up_output(model);
		// Remove it from the schedule
		sched.schedule(model,DBL_MAX);
	}
	// Otherwise try to find a thread for it
	else
	{
		model->lp->needs_update = true; 
		// Look for an available thread to do the calculation
		for (int i = 0; i < thread_count && dt > 0.0; i++)
		{
			if (i != SAFE_THREAD)
			{
				LogicalProcess<X>** lp_ptr = &(pending[i]);
				#pragma omp flush(lp_ptr)
				if (*lp_ptr == NULL)
				{
					prep_for_speculation(model);
					#pragma omp flush
					bool* safe_ptr = &(model->lp->is_safe);
					*safe_ptr = false;
					*lp_ptr = model->lp;
					#pragma omp flush(lp_ptr,safe_ptr)
					model->lp->needs_update = false;
					break;
				}
			}
		}
		if (model->lp->needs_update)
			clean_up_output(model);
	}
	if (dt < DBL_MAX)
	{
		if (0.0 < dt)
			sched.schedule(model,Time(t.t+dt,0));
		else
			sched.schedule(model,Time(t.t,t.c+1));
	}
	// Return the input bag to the pool
	if (model->x != NULL)
	{
		model->x->clear();
		io_pool.destroy_obj(model->x);
		model->x = NULL;
	}
	// Remember the time of the last event
	model->tL = t;
	// Clear the active flag
	model->active = false;
}

template <class X>
void OptSimulator<X>::route(Network<X>* parent, Devs<X>* src, X& x, int thrd,
		Bag<Event<X> >* target, Bag<Event<X> >* output)
{
	// Notify event listeners if this is an output event
	if (parent != src)
	{
		if (output == NULL)
			notify_output_listeners(src,x,sched.minPriority().t);
		else
			output->insert(Event<X>(src,x));
	}
	// No one to do the routing, so return
	if (parent == NULL) return;
	// Compute the set of receivers for this value
	Bag<Event<X> >* recvs = recv_pool[thrd].make_obj();
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
			if (target == NULL) inject_event(amodel,(*recv_iter).value);
			else target->insert(*recv_iter);
		}
		// if this is an external output from the parent model
		else if ((*recv_iter).model == parent)
		{
			route(parent->getParent(),parent,(*recv_iter).value,thrd,target,output);
		}
		// otherwise it is an input to a coupled model
		else
		{
			route((*recv_iter).model->typeIsNetwork(),
			(*recv_iter).model,(*recv_iter).value,thrd,target,output);
		}
	}
	recvs->clear();
	recv_pool[thrd].destroy_obj(recvs);
}

template <class X>
void OptSimulator<X>::inject_event(Atomic<X>* model, X& value)
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
void OptSimulator<X>::exec_event(Atomic<X>* model, bool internal, Time t)
{
	// If we already have a state for this model
	if (model->lp->checkpoint != NULL)
	{
		bool good = false;
		// If the state is good, then we are done
		if (model->x == NULL)
		{
			notify_state_listeners(model,t.t);
			good = true;
		}
		// Otherwise restore the state to the last event time
		else
		{
			model->restore_state(model->lp->checkpoint);
		}
		// Remove the checkpoint
		model->gc_state(model->lp->checkpoint);
		model->lp->checkpoint = NULL;
		// If it was a good checkpoint, then we are done
		if (good) return;
	}
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
		model->delta_ext(t.t-model->tL.t,*(model->x));
	}
	// Notify any listeners
	notify_state_listeners(model,t.t);
}

} // End of namespace

#endif
