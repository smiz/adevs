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
		bool is_safe;
		void* checkpoint;
		Bag<Event<X> > targets, outputs; 
		int wait_for_safe()
		{
			int stalled = 0;
			bool* is_safe_ptr = &is_safe;
			while (!(*is_safe_ptr))
			{
				stalled = 1;
				#pragma omp flush(is_safe_ptr)
			}
			#pragma omp flush
			return stalled;
		}
};

template <typename X> class SpecThread
{
	public:
		SpecThread(){ model = NULL; }
		void execute(bool* halt, SpecThread<X>** ready);
		// The assigned model must have a valid LP
		void startWork(Atomic<X>* work)
		{
			Atomic<X>** model_ptr = &model;
			bool* safe_ptr = &(work->lp->is_safe);
			*safe_ptr = false;
			*model_ptr = work;
			#pragma omp flush(model_ptr,safe_ptr)
		}
		bool isIdle()
		{
			Atomic<X>** model_ptr = &model;
			#pragma omp flush(model_ptr)
			return (*model_ptr == NULL);
		}
		~SpecThread(){}
	private:
		object_pool<Bag<Event<X> > > recv_pool;
		Atomic<X>* model;
		void route(Network<X>* parent, Devs<X>* src, X& x);
}; 

template <class X>
void SpecThread<X>::execute(bool* halt, SpecThread<X>** ready)
{
	for (;;)
	{
		Atomic<X>** model_ptr = &model;
		while (!(*halt) && (*model_ptr) == NULL)
		{
			*ready = this;
			#pragma omp flush(halt,model_ptr,ready)
		}
		if (*halt) return;
		#pragma omp flush
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
				y_iter != model->y->end(); y_iter++)
		{
			route(model->getParent(),model,*y_iter);
		}
		// Speculate on the state
		if ((model->lp->checkpoint = model->save_state()) != NULL)
		{
			model->delta_int();
		}
		#pragma omp flush
		bool* safe_ptr = &(model->lp->is_safe);
		*safe_ptr = true;
		#pragma omp flush(safe_ptr)
		*model_ptr = NULL;
		#pragma omp flush(model_ptr)
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
