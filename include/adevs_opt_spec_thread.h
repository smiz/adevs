#ifndef __adevs_opt_spec_thread_h_
#define __adevs_opt_spec_thread_h_
#include "adevs_models.h"
#include "object_pool.h"
#include <cstdlib>
#include <omp.h>

namespace adevs
{

template <typename X> class LogicalProcess
{
	public:
		LogicalProcess()
		{
			checkpoint = NULL;
			is_safe = true;
		}
		void* checkpoint;
		Bag<Event<X> > targets, outputs; 
		int wait_for_safe()
		{
			int stalls = 0;
			while (!is_safe)
			{
				stalls = 1;
			}
			{
				#pragma omp flush
			}
			return stalls;
		}
		void set_safe(bool safe)
		{
			is_safe = safe;
		}
		~LogicalProcess()
		{
		}
		void set_interrupt() { stop_work = true; }
		void clear_interrupt() { stop_work = false; }
		bool interrupt() { return stop_work; }
	private:
		volatile bool is_safe, stop_work;
};

template <typename X> class SpecThread
{
	public:
		SpecThread()
		{
			model = NULL;
			run = true;
			has_model = false;
		}
		void execute();
		// The assigned model must have a valid LP
		void startWork(Atomic<X>* work)
		{
			model = work;
			work->lp->set_safe(false);
			work->lp->clear_interrupt();
			{
				#pragma omp flush
			}
			has_model = true;
		}
		void stop()
		{
			run = false;
		}
		bool isIdle() 
		{
			return !has_model;
		}
		~SpecThread()
		{
		}
	private:
		object_pool<Bag<Event<X> > > recv_pool;
		Atomic<X> *model;
		volatile bool has_model, run;
		void route(Network<X>* parent, Devs<X>* src, X& x);
}; 

template <class X>
void SpecThread<X>::execute()
{
	for (;;)
	{
		while (!has_model && run);
		{
			#pragma omp flush
		}
		if (model == NULL)
		{
			run = true;
			return;
		}
		if (!model->y->empty())
		{
			model->gc_output(*(model->y));
			model->y->clear();
		}
		model->lp->outputs.clear();
		model->lp->targets.clear();
		// Speculate on a value
		model->output_func(*(model->y));
		// Route each event in y
		for (typename Bag<X>::iterator y_iter = model->y->begin(); 
				y_iter != model->y->end() && !model->lp->interrupt(); y_iter++)
		{
			route(model->getParent(),model,*y_iter);
		}
		// Speculate on the state
		if (!model->lp->interrupt())
		{
			if ((model->lp->checkpoint = model->save_state()) != NULL)
				model->delta_int();
		}
		{
			#pragma omp flush
		}
		model->lp->set_safe(true);
		model = NULL;
		has_model = false;
	}
}
	
template <class X>
void SpecThread<X>::route(Network<X>* parent, Devs<X>* src, X& x)
{
	// Notify event listeners if this is an output event
	if (parent != src)
	{
		model->lp->outputs.insert(Event<X>(src,x));
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
			model->lp->targets.insert(*recv_iter);
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

} // End of namespace

#endif
