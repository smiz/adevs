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

template <typename X> class LogicalProcess:
	public ulist<LogicalProcess<X> >::one_list
{
	public:
		LogicalProcess():
			ulist<LogicalProcess<X> >::one_list()
		{
			omp_init_lock(&lock);
			checkpoint = NULL;
			is_pending = false;
			spec = true;
			ta = DBL_MAX;
			targets = outputs = NULL;
		}
		double ta;
		bool spec;
		void* checkpoint;
		bool is_pending;
		Atomic<X>* model;
		Bag<Event<X> > *targets, *outputs; 
		void Lock() { omp_set_lock(&lock); }
		void Unlock() { omp_unset_lock(&lock); }
		bool TryLock() { return omp_test_lock(&lock); }
		~LogicalProcess() { omp_destroy_lock(&lock); }
	private:
		omp_lock_t lock;
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
	private:
		/// The event schedule
		Schedule<X,Time> sched;
		/// List of imminent models
		Bag<Atomic<X>*> imm;
		/// List of models activated by input
		Bag<Atomic<X>*> activated;
		bool halt;
		omp_lock_t pending_lock;
		unsigned early_output, pending_size;
		ulist<LogicalProcess<X> > pending;
		/// Pools of preallocated, commonly used objects
		object_pool<Bag<X> > io_pool;
		object_pool<Bag<Event<X> > >* recv_pool;

		void inject_event(Atomic<X>* model, X& value);	
		void exec_event(Atomic<X>* model, bool internal, Time t);
		void schedule(Atomic<X>* model, Time t);
		void init(Devs<X>* model);
		void route(Network<X>* parent, Devs<X>* src, X& x, int thrd_id,
				Bag<Event<X> >* targets = NULL, Bag<Event<X> >* outputs = NULL);
		void speculate(int thread_num);
		void simSafe(double tend);
		void cleanup(Atomic<X>* model);
}; 

template <class X>
OptSimulator<X>::OptSimulator(Devs<X>* model):
	AbstractSimulator<X>()
{
	int thread_count = omp_get_max_threads();
	recv_pool = new object_pool<Bag<Event<X> > >[thread_count];
	pending_size = early_output = 0;
	omp_init_lock(&pending_lock);
	init(model);
}

template <class X>
OptSimulator<X>::~OptSimulator<X>()
{
	omp_destroy_lock(&pending_lock);
}

template <class X>
void OptSimulator<X>::execUntil(double stop_time)
{
	int i;
	halt = false;
	int thread_count = omp_get_max_threads();
	#pragma omp parallel for default(shared) private(i)
	for (i = 0; i < thread_count; i++)
	{
		if (i == 0) simSafe(stop_time);
		else speculate(i);
	}
}

template <class X>
void OptSimulator<X>::speculate(int thread_num)
{
	while (true)
	{
		while (!halt && pending_size == 0)
		{
			#pragma omp flush(halt,pending_size)
		}
		if (halt) return;
		// Find a model in the schedule that does not have a checkpoint
		LogicalProcess<X>* lp = NULL;
		omp_set_lock(&pending_lock);
	   	if (!pending.empty())
		{
			pending_size--;
			lp = pending.back();
			pending.pop_back();
			lp->is_pending = false;
		}
		omp_unset_lock(&pending_lock);
		if (lp != NULL && lp->TryLock())
		{
			// If it has not had an output computed yet
			if (lp->ta < DBL_MAX && lp->model->x == NULL && lp->spec)
			{
				lp->spec = false;
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
			} 
			lp->Unlock();
		}
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
			// If the output for this model has not already been computed,
			// then get it
			model->lp->Lock();
			if (model->lp->spec)
			{
				model->lp->spec = false;
				model->output_func(*(model->y));
				// Route each event in y
				for (typename Bag<X>::iterator y_iter = model->y->begin(); 
					y_iter != model->y->end(); y_iter++)
				{
					route(model->getParent(),model,*y_iter,0);
				}
			}
			else
			{
				early_output++;
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
			model->lp->Unlock();
		}
		// Compute new states for the imminent and active models and put
		// them back into the schedule
		for (typename Bag<Atomic<X>*>::iterator iter = imm.begin(); 
			iter != imm.end(); iter++)
		{
			(*iter)->lp->Lock();
			exec_event(*iter,true,t); // Internal and confluent transitions
			(*iter)->lp->Unlock();
		}
		for (typename Bag<Atomic<X>*>::iterator iter = activated.begin(); 
			iter != activated.end(); iter++)
		{
			(*iter)->lp->Lock();
			exec_event(*iter,false,t); // External transitions
			cleanup(*iter);
			schedule(*iter,t);
			(*iter)->lp->spec = true;
			(*iter)->lp->Unlock();
		}
		// Clean up and reschedule the imminent models
		for (typename Bag<Atomic<X>*>::iterator iter = imm.begin(); 
			iter != imm.end(); iter++)
		{
			(*iter)->lp->Lock();
			cleanup(*iter);
			schedule(*iter,t);
			(*iter)->lp->spec = true;
			(*iter)->lp->Unlock();
		}
		imm.clear();
		activated.clear();
	}
}

template <class X>
void OptSimulator<X>::cleanup(Atomic<X>* model)
{
	if (model->x != NULL)
	{
		model->x->clear();
		io_pool.destroy_obj(model->x);
		model->x = NULL;
	}
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
	if (model->lp->targets != NULL)
	{
		model->lp->outputs->clear();
		model->lp->targets->clear();
		recv_pool[0].destroy_obj(model->lp->targets);
		recv_pool[0].destroy_obj(model->lp->outputs);
		model->lp->targets = NULL;
	}
	model->active = false;
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
void OptSimulator<X>::schedule(Atomic<X>* a, Time t)
{
	a->tL = t;
	double dt = a->lp->ta = a->ta();
	if (dt < 0.0)
	{
		exception err("Negative time advance",a);
		throw err;
	}
	if (omp_test_lock(&pending_lock))
	{
		if (a->lp->is_pending && dt == DBL_MAX)
		{
			pending_size--;
			pending.erase(a->lp);
			a->lp->is_pending = false;
		}
		else if (!a->lp->is_pending && dt < DBL_MAX)
		{
			pending_size++;
			pending.push_back(a->lp);
			a->lp->is_pending = true;
		}
		omp_unset_lock(&pending_lock);
	}
	if (dt == DBL_MAX)
	{
		sched.schedule(a,DBL_MAX);
		return;
	}
	if (0.0 < dt)
		sched.schedule(a,Time(t.t+dt,0));
	else
		sched.schedule(a,Time(t.t,t.c+1));
	a->y = io_pool.make_obj();
	a->lp->targets = recv_pool[0].make_obj();
	a->lp->outputs = recv_pool[0].make_obj();
}

template <class X>
void OptSimulator<X>::route(Network<X>* parent, Devs<X>* src, X& x, int thrd, Bag<Event<X> >* target, Bag<Event<X> >* output)
{
	// Notify event listeners if this is an output event
	if (parent != src)
	{
		if (target == NULL) notify_output_listeners(src,x,sched.minPriority().t);
		else output->insert(Event<X>(src,x));
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
