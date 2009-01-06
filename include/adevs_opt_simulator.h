#ifndef __adevs_opt_simulator_h_
#define __adevs_opt_simulator_h_
#include "adevs.h"
#include "adevs_abstract_simulator.h"
#include "adevs_opt_spec_thread.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <pthread.h>
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
		OptSimulator(Devs<X>* model, int num_spec_threads = 1);
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
		struct thread_data_t	
		{
			pthread_t thrd;
			SpecThread<X>* spec;
		};
		/// The event schedule
		Schedule<X,Time> sched;
		/// List of imminent models
		Bag<Atomic<X>*> imm;
		/// List of models activated by input
		Bag<Atomic<X>*> activated;
		unsigned early_output, jit_output, ext_stalls, int_stalls;
		/// Pools of preallocated, commonly used objects
		object_pool<Bag<X> > io_pool;
		object_pool<Bag<Event<X> > > recv_pool;
		object_pool<LogicalProcess<X> > lp_pool;
		/// Speculative threads
		thread_data_t* spec_thrd;
		unsigned next_thrd;
		const int thread_count;
		void inject_event(Atomic<X>* model, X& value);	
		void exec_event(Atomic<X>* model, bool internal, Time t);
		void schedule(Atomic<X>* model, Time t);
		void init(Devs<X>* model);
		void route(Network<X>* parent, Devs<X>* src, X& x);
		void cleanup(Atomic<X>* model);
	
		static void* start_thread(void* thread_data)
		{
			thread_data_t* data = static_cast<thread_data_t*>(thread_data);
			data->spec->execute();
			return NULL;
		}
}; 

template <class X>
OptSimulator<X>::OptSimulator(Devs<X>* model, int num_spec_threads):
	AbstractSimulator<X>(),
	thread_count(num_spec_threads)
{
	next_thrd = 0;
	spec_thrd = new thread_data_t[thread_count];
	for (int i = 0; i < thread_count; i++)
	{
		spec_thrd[i].spec = new SpecThread<X>();
		pthread_create(&(spec_thrd[i].thrd),NULL,start_thread,&(spec_thrd[i]));
	}
	jit_output = ext_stalls = int_stalls = early_output = 0;
	init(model);
}

template <class X>
OptSimulator<X>::~OptSimulator<X>()
{
	// Delete the speculating threads
	for (int i = 0; i < thread_count; i++)
	{
		spec_thrd[i].spec->stop();
		pthread_join(spec_thrd[i].thrd,NULL);
		delete spec_thrd[i].spec;
	}
	delete [] spec_thrd;
	// Cleanup all of the models in the schedule
	for (unsigned i = 1; i <= sched.getSize(); i++)
	{
		Atomic<X>* model = sched.get(i);
		if (model->lp != NULL && model->lp->checkpoint != NULL)
			model->gc_state(model->lp->checkpoint);
		cleanup(sched.get(i));
	}
	// Clean up any lingering activated models
	for (typename Bag<Atomic<X>*>::iterator iter = activated.begin();
			iter != activated.end(); iter++)
		cleanup(*iter);
}

template <class X>
void OptSimulator<X>::execUntil(double tstop)
{
	Time t;
	while (true)
	{
		// Find the imminent models and the next event time
		t = sched.minPriority();
		if (tstop < t.t || t.t == DBL_MAX)
		{
			return;
		}
		sched.getImminent(imm);
		// Compute output and route events
		for (typename Bag<Atomic<X>*>::iterator imm_iter = imm.begin(); 
			imm_iter != imm.end(); imm_iter++)
		{
			Atomic<X>* model = *imm_iter;
			// If it has not had an output computed yet
			if (model->lp == NULL)
			{
				model->y = io_pool.make_obj();
				jit_output++;
				// Compute its output
				model->output_func(*(model->y));
				// Route each event in y
				for (typename Bag<X>::iterator y_iter = model->y->begin(); 
					y_iter != model->y->end(); y_iter++)
				{
					route(model->getParent(),model,*y_iter);
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
				for (typename Bag<Event<X> >::iterator y_iter = model->lp->targets.begin();
						y_iter != model->lp->targets.end(); y_iter++)
				{
					inject_event((*y_iter).model->typeIsAtomic(),(*y_iter).value);
				}
				for (typename Bag<Event<X> >::iterator y_iter = model->lp->outputs.begin();
						y_iter != model->lp->outputs.end(); y_iter++)
				{
					notify_output_listeners((*y_iter).model,(*y_iter).value,sched.minPriority().t);
				}
			}
		}
		for (typename Bag<Atomic<X>*>::iterator iter = imm.begin(); 
			iter != imm.end(); iter++)
		{
			exec_event(*iter,true,t); // Internal and confluent transitions
		}
		for (typename Bag<Atomic<X>*>::iterator iter = activated.begin(); 
			iter != activated.end(); iter++)
		{
			if ((*iter)->lp != NULL)
			{
				ext_stalls += (*iter)->lp->wait_for_safe();
			}
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
		a->lp = NULL;
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
void OptSimulator<X>::cleanup(Atomic<X>* model)
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
	if (model->lp != NULL)
	{
		lp_pool.destroy_obj(model->lp);
		model->lp = NULL;
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
		// Clean up its output and lp
		cleanup(model);
		// Remove it from the schedule
		sched.schedule(model,DBL_MAX);
	}
	// Otherwise try to find a thread for it
	else
	{
		SpecThread<X>* to_use = NULL;
		if (thread_count > 0 && dt > 0.0)
			to_use = spec_thrd[(++next_thrd)%thread_count].spec;
		// Look for an available thread to do the calculation
		if (to_use != NULL && to_use->isIdle())
		{
			if (model->y == NULL)
				model->y = io_pool.make_obj();
			if (model->lp == NULL)
				model->lp = lp_pool.make_obj();
			to_use->startWork(model);
		}
		else
		{
			cleanup(model);
		}
		// Put the model into the schedule
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
void OptSimulator<X>::route(Network<X>* parent, Devs<X>* src, X& x)
{
	// Notify event listeners if this is an output event
	if (parent != src)
	{
		notify_output_listeners(src,x,sched.minPriority().t);
	}
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
void OptSimulator<X>::inject_event(Atomic<X>* model, X& value)
{
	if (model->active == false)
	{
		if (model->lp != NULL)
			model->lp->set_interrupt();
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
	if (model->lp != NULL && model->lp->checkpoint != NULL)
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
