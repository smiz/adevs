#ifndef __adevs_opt_spec_thread_h_
#define __adevs_opt_spec_thread_h_
#include "adevs_models.h"
#include "object_pool.h"
#include <cstdlib>
#include <pthread.h>

namespace adevs
{

template <typename X> class LogicalProcess
{
	public:
		LogicalProcess()
		{
			pthread_mutex_init(&mtx,NULL);
			pthread_cond_init(&cond,NULL);
			checkpoint = NULL;
			is_safe = true;
		}
		void* checkpoint;
		Bag<Event<X> > targets, outputs; 
		int wait_for_safe()
		{
			int stalls = 0;
			pthread_mutex_lock(&mtx);
			while (!is_safe)
			{
				stalls = 1;
				pthread_cond_wait(&cond,&mtx);
			}
			pthread_mutex_unlock(&mtx);
			return stalls;
		}
		void set_safe_fast(bool safe)
		{
			is_safe = safe;
		}
		void set_safe(bool safe)
		{
			pthread_mutex_lock(&mtx);
			is_safe = safe;
			pthread_cond_signal(&cond);
			pthread_mutex_unlock(&mtx);
		}
		~LogicalProcess()
		{
			pthread_mutex_destroy(&mtx);
			pthread_cond_destroy(&cond);
		}
		void set_interrupt() { stop_work = true; }
		void clear_interrupt() { stop_work = false; }
		bool interrupt() const { return stop_work; }
	private:
		bool is_safe, stop_work;
		pthread_cond_t cond;
		pthread_mutex_t mtx;
};

template <typename X> class SpecThread
{
	public:
		SpecThread()
		{
			pthread_mutex_init(&mtx,NULL);
			pthread_cond_init(&cond,NULL);
			pending = model = NULL;
			run = true;
		}
		void execute();
		// The assigned model must have a valid LP
		void startWork(Atomic<X>* work)
		{
			work->lp->set_safe_fast(false);
			work->lp->clear_interrupt();
			pthread_mutex_lock(&mtx);
			pending = work;
			pthread_cond_signal(&cond);
			pthread_mutex_unlock(&mtx);
		}
		void stop()
		{
			pthread_mutex_lock(&mtx);
			run = false;
			pthread_cond_signal(&cond);
			pthread_mutex_unlock(&mtx);
		}
		bool isIdle() const
		{
			return (pending == NULL);
		}
		~SpecThread()
		{
			pthread_mutex_destroy(&mtx);
			pthread_cond_destroy(&cond);
		}
	private:
		object_pool<Bag<Event<X> > > recv_pool;
		Atomic<X> *model, *pending;
		pthread_cond_t cond;
		pthread_mutex_t mtx;
		bool run;
		void route(Network<X>* parent, Devs<X>* src, X& x);
}; 

template <class X>
void SpecThread<X>::execute()
{
	for (;;)
	{
		pthread_mutex_lock(&mtx);
		while (pending == NULL && run)
		{
			pthread_cond_wait(&cond,&mtx);
		}
		if (!run)
		{
			run = true;
			pthread_mutex_unlock(&mtx);
			return;
		}
		model = pending;
		pending = NULL;
		pthread_mutex_unlock(&mtx);
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
		model->lp->set_safe(true);
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
