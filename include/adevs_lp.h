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
#ifndef __adevs_lp_h_
#define __adevs_lp_h_
#include "adevs.h"
#include "adevs_abstract_simulator.h"
#include "adevs_time.h"
#include "object_pool.h"
#include <list>
#include <typeinfo>
#include <omp.h>

/**
 * This is an implementation of the time warp simulation algorithm described in
 * J. Nutaro, "On Constructing Optimistic Simulation Algorithms for the Discrete
 * Event System Specification", Transactions on Modeling and Computer Simulation,
 * Vol. V, No. N, pgs xx--yy, 2008.
 *
 * This file contains all of the support classes for the optimistic simulator
 * in adevs_opt_sim.h
 */
namespace adevs
{

// A simulation message
template <class X> struct Message
{
	// The message timestamp
	Time t;
	// The model produced and that will receive the message
	Atomic<X> *src, *dst;
	// The value of the message
	X value;
	// LP unique message ID. Negative numbers are anti-messages, positive
	// numbers are inter-LP messages. Zero is reserved to indicate the
	// actual output from the model.
	long int ID;
};

struct LP_perf_t
{
	unsigned rollbacks;
	unsigned canceled_output, canceled_intra_lp_output;
	LP_perf_t():rollbacks(0),canceled_output(0),canceled_intra_lp_output(0){}
};

/*
 * A logical process is assigned to every atomic model and it simulates
 * that model optimistically. The atomic model must support state saving
 * and state restoration.
 */
template <class X> class LogicalProcess
{
	public:

		/**
		 * Constructor builds a logical process without any models
		 * assigned to it.
		 */
		LogicalProcess();
		/**
		 * Assign a model to this logical process.
		 */
		void addModel(Atomic<X>* model);
		/**
		 * Do fossil collection and report correct states and outputs.
		 */
		void fossilCollect(Time gvt, AbstractSimulator<X>* sim);
		/**
		 * Send a message to the logical process. This will put the message into the
		 * back of the input queue.
		 */
		void sendMessage(Message<X>& m);
		/**
		 * Get the smallest of the local time of next event. For this to be used in a gvt calculation
		 * the input lists need to be flushed by calling processInput for all LPs until they return
		 * false for pendingInput()
		 */
		Time getNextEventTime() const
		{
			Time result = sched.minPriority();
			if (!avail.empty() && avail.front().t < result)
				result = avail.front().t;
			return result;
		}
		/** 
		 * Process all of the events in the input list. 
		 */
		void processInput();
		/**
		 * Optimistically execute the output and state transition functions.
		 */
		void execEvents();
		/**
		 * Is there unprocessed input in the input list?
		 */
		bool pendingInput() { return !input.empty(); }
		/**
		 * Get performance information for the LP
		 */
		LP_perf_t getPerfData() const { return perf_data; }
		/**
		 * Destructor leaves the models intact.
		 */
		~LogicalProcess();
	private:
		// A state checkpoint
		struct CheckPoint
		{
			// Initial time and final time for the checkpoint state
			Time ti, tf;
			// Model that this is a checkpoint for
			Atomic<X>* model;
			// Pointer to the saved state
			void* data;
		};
		// Structure for tracking performance related data
		LP_perf_t perf_data;
		// Last commit timestamp
		Time tCommit;
		// List of input messages
		std::list<Message<X> > input;
		// lock variable for the input list
		omp_lock_t lock;
		// Time ordered list of messages that are available for processing
		std::list<Message<X> > avail;
		// Time ordered list of messages that have been processed
		std::list<Message<X> > used;
		// Time ordered list of checkpoints
		std::list<CheckPoint> chk_pt;
		// Time ordered list of good output messages
		std::list<Message<X> > output;
		// Time ordered list of discarded output messages
		std::list<Message<X> > discard;
		// Bags of imminent and active models.
		Bag<Atomic<X>*> imm, activated;
		/// The event schedule
		Schedule<X,Time> sched;
		/// Pools of preallocated, commonly used objects
		object_pool<Bag<X> > io_pool;
		object_pool<Bag<Event<X> > > recv_pool;
		/// Counter for producing message identifiers
		long int ID;
		/// Can route send messages between LPs?
		bool inter_LP_ok;
		// Insert a message into a timestamp ordered list
		void insert_message(std::list<Message<X> >& l, Message<X>& msg);
		// Route events using the Network models' route methods
		void route(Network<X>* parent, Devs<X>* src, X& x);
		// Save state and compute state transition of an atomic model
		void exec_event(Atomic<X>* model, bool internal, Time t);
		// Inject an input into an atomic model
		void inject_event(Atomic<X>* model, X& value);
		// Destroy a message in the list l with the antimessage msg
		void anti_message(std::list<Message<X> >& l, const Message<X>& msg);
};

template <class X>
LogicalProcess<X>::LogicalProcess()
{
	// Do not report initial states
	tCommit = Time(0.0,1);
	// Message ID counter starts at 1
	ID = 1;
	// Create the lock for the input list
	omp_init_lock(&lock);	
}

template <class X>
void LogicalProcess<X>::addModel(Atomic<X>* model)
{
	// Assign the model to this LP
	model->lp = this;
	// Put it into the schedule
	double dt = model->ta();
	if (dt < DBL_MAX) sched.schedule(model,Time(dt,0));
}

template <class X>
void LogicalProcess<X>::fossilCollect(Time gvt, AbstractSimulator<X>* sim)
{
	// Models that need to report their current state
	std::set<Atomic<X>*> active;
	// Report and delete old states
	typename std::list<CheckPoint>::iterator chk_pt_iter = chk_pt.begin();
	while (chk_pt_iter != chk_pt.end())
	{
		Atomic<X>* model = (*chk_pt_iter).model;
		void* data = (*chk_pt_iter).data;
		Time ti = (*chk_pt_iter).ti;
		Time tf = (*chk_pt_iter).tf;
		// Report states that are good
		if (sim != NULL)
		{
			if (model->tL < gvt && tCommit <= model->tL)
				active.insert(model);
			if (ti < gvt && tCommit <= ti)
				sim->notify_state_listeners(model,ti.t,data);
		}
		// Delete states that are no longer needed
		if (tf < gvt)
		{
			model->gc_state(data);
			chk_pt_iter = chk_pt.erase(chk_pt_iter);
		}
		else chk_pt_iter++;
	}
	// Report the active models
	typename std::set<Atomic<X>*>::iterator active_iter = active.begin();
	for (; active_iter != active.end(); active_iter++)
		sim->notify_state_listeners(*active_iter,(*active_iter)->tL.t);
	if (tCommit < gvt) tCommit = gvt;
	// Delete old used messages
	while (!used.empty() && used.front().t < gvt)
		used.pop_front();
	// Delete old output
	Bag<X>* garbage = io_pool.make_obj();
	while (!discard.empty() && discard.front().t < gvt) 	
	{
		garbage->insert(discard.front().value);
		discard.front().src->gc_output(*garbage);
		garbage->clear();
		discard.pop_front();
	}
	while (!output.empty() && output.back().t < gvt) 
	{
		Message<X> msg = output.back();
		if (msg.ID == 0)
		{
			if (sim != NULL)
				sim->notify_output_listeners(msg.src,msg.value,msg.t.t);
			garbage->insert(msg.value);
			msg.src->gc_output(*garbage);
			garbage->clear();
		}
		output.pop_back();
	}
	io_pool.destroy_obj(garbage);
}

template <class X>
void LogicalProcess<X>::anti_message(std::list<Message<X> >& l, const Message<X>& msg)
{
	typename std::list<Message<X> >::iterator msg_iter = l.begin();
	while (msg_iter != l.end())
	{
		if (msg.ID == -(*msg_iter).ID && msg.src->lp == (*msg_iter).src->lp)
		{
			l.erase(msg_iter);
			return;
		}
		else msg_iter++;
	}
}

template <class X>
void LogicalProcess<X>::processInput()
{
	// Process all of the input messages. This method
	// performs rollbacks and message cancellations as
	// required.
	for (;;) // While there are input in the list
	{
		// Get the message at the front of the list or exit if no message is available
		bool no_input;
		Message<X> msg;
		omp_set_lock(&lock);
		if ((no_input = input.empty()) == false)
		{
			msg = input.front();
			input.pop_front();
		}
		omp_unset_lock(&lock);
		if (no_input) return;
		// If this is a message in the past
		if (!chk_pt.empty() && msg.t <= chk_pt.back().tf)
		{
			assert(avail.empty() || chk_pt.back().tf <= avail.front().t);
			// Cancel inter-LP output that occurred after the message time and
			// intra-LP output that occurred at or after the message time
			typename std::list<Message<X> >::iterator msg_iter = output.begin();	
			while (msg_iter != output.end())
			{
				// Send an anti-message to cancel inter-LP communications
				if (0 < (*msg_iter).ID && msg.t < (*msg_iter).t)
				{
					(*msg_iter).ID *= -1;
					(*msg_iter).dst->lp->sendMessage(*msg_iter);
					msg_iter = output.erase(msg_iter);
					perf_data.canceled_output++;
				}
				// Discard an intra-LP communication
				else if (0 == (*msg_iter).ID && msg.t <= (*msg_iter).t)
				{
					insert_message(discard,*msg_iter);
					msg_iter = output.erase(msg_iter);
					perf_data.canceled_intra_lp_output++;
				}
				// If the message is in the past then we are done
				else if ((*msg_iter).t < msg.t) break;
				// Otherwise move to the next one
				else msg_iter++;
			}
			// If it is a rollback message, then remove it from the used list
			if (msg.ID < 0) anti_message(used,msg);
			// Move used messages at and after the event back to the available list
			while (!used.empty() && msg.t <= used.back().t)
			{
				avail.push_front(used.back());
				used.pop_back();
			}
			// Restore to a checkpoint that is in the past
			while (!chk_pt.empty() && msg.t <= chk_pt.back().tf)
			{	
				Atomic<X>* model = chk_pt.back().model;
				// Restore the state
				void* data = chk_pt.back().data;
				model->tL = chk_pt.back().ti;
				model->restore_state(data);
				// Reposition the model in the schedule
				double h = model->ta();
				if (h < DBL_MAX)
				{
					Time tN = model->tL+Time(h,0);
					if (tN < model->tL) tN = model->tL;
					sched.schedule(model,tN);
				}
				else sched.schedule(model,Time::Inf());
				// Delete the checkpoint
				model->gc_state(data);
				chk_pt.pop_back();
				perf_data.rollbacks++;
			}
		}
		// Otherwise if it is a rollback message in the future
		// so just remove it from the available list
		else if (msg.ID < 0) 
		{
			anti_message(avail,msg);
		}
		// If it is not a rollback message, then put it into the available list
		if (msg.ID > 0) insert_message(avail,msg);
	}
	assert(avail.empty() || used.empty() || used.back().t < avail.front().t);
	assert(!chk_pt.empty() || used.empty());
}

template <class X>
void LogicalProcess<X>::execEvents()
{
	Time tN = getNextEventTime();
	// If the next event is at infinity, then there is nothing to do
	if (tN.t == DBL_MAX) return;
	// Get the imminent models and route their output
	if (sched.minPriority() <= tN)
	{
		// Get the imminent messages
		sched.getImminent(imm);
		// Compute and send the output values
		Message<X> msg;
		msg.t = tN;
		// Is this message a replay of an earlier message?
		inter_LP_ok = (output.empty() || output.front().t < tN);
		for (typename Bag<Atomic<X>*>::iterator imm_iter = imm.begin(); 
			imm_iter != imm.end(); imm_iter++)
		{
			Atomic<X>* model = *imm_iter;
			msg.ID = 0;
			msg.src = model;
			model->y = io_pool.make_obj();
			model->output_func(*(model->y));
			// Route each event in y
			for (typename Bag<X>::iterator y_iter = model->y->begin(); 
				y_iter != model->y->end(); y_iter++)
			{
				// Local copy of the output
				msg.value = *y_iter;
				output.push_front(msg);
				// Send messages to local models and other LPs
				route(model->getParent(),model,*y_iter);
			}
			// Clean up
			model->y->clear();
			io_pool.destroy_obj(model->y);
		}
	}
	// Route the external input that is in the avail list
	while (!avail.empty() && avail.front().t <= tN)
	{
		Message<X> msg = avail.front();
		avail.pop_front();
		used.push_back(msg);
		inject_event(msg.dst->typeIsAtomic(),msg.value);
	}
	// Compute the next states of all of the actived and imminent models
	typename Bag<Atomic<X>*>::iterator iter;
	for (iter = imm.begin(); iter != imm.end(); iter++)
		exec_event(*iter,true,tN);
	for (iter = activated.begin(); iter != activated.end(); iter++)
		exec_event(*iter,false,tN);
	imm.clear();
	activated.clear();
}

template <class X>
void LogicalProcess<X>::exec_event(Atomic<X>* model, bool internal, Time t)
{
	// Save the current state 
	CheckPoint c;
//	std::cerr << typeid(*model).name() << " save state " << t << std::endl;
	c.ti = model->tL;
	c.tf = t;
	c.model = model;
	c.data = model->save_state();
	chk_pt.push_back(c); 
	// Compute the next state
	if (model->x == NULL) model->delta_int();
	else
	{
		if (internal) model->delta_conf(*(model->x));
		else model->delta_ext(t.t-model->tL.t,*(model->x));
		model->x->clear();
		io_pool.destroy_obj(model->x);
		model->x = NULL;
	}
	model->tL = t+Time(0.0,1);
	double dt = model->ta();
	if (dt < DBL_MAX)
	{
		Time tN = model->tL + Time(dt,0);
		if (tN < model->tL) tN = model->tL;
		sched.schedule(model,tN);
	}
	else sched.schedule(model,Time::Inf());
	model->active = false;
}

template <class X>
void LogicalProcess<X>::inject_event(Atomic<X>* model, X& value)
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
void LogicalProcess<X>::route(Network<X>* parent, Devs<X>* src, X& x)
{
	// No one to do the routing, so return
	if (parent == NULL) return;
	// Create a bag to collect the receivers
	Bag<Event<X> >* recvs = recv_pool.make_obj();
	// Compute the set of receivers for this value
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
		if the destination is an atomic model
		*/
		amodel = (*recv_iter).model->typeIsAtomic();
		if (amodel != NULL)
		{
			// Atomic model local to the LP
			if (amodel->lp == this)
				inject_event(amodel,(*recv_iter).value);
			// Atomic model at another LP
			else if (inter_LP_ok)
			{
				Message<X> m = output.front();
				m.ID = ID++;
				assert(ID > 0);
				m.dst = amodel;
				assert(m.src != m.dst);
				m.value = (*recv_iter).value;
				amodel->lp->sendMessage(m); 
				output.push_front(m);
			}
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
	// Free the bag of receivers
	recvs->clear();
	recv_pool.destroy_obj(recvs);
}

template <class X>
void LogicalProcess<X>::sendMessage(Message<X>& msg)
{
	// Insert the message
	omp_set_lock(&lock);
	input.push_back(msg);
	omp_unset_lock(&lock);
}

template <class X>
void LogicalProcess<X>::insert_message(std::list<Message<X> >& l, Message<X>& msg)
{
	typename std::list<Message<X> >::iterator msg_iter;
	for (msg_iter = l.begin(); msg_iter != l.end(); msg_iter++)
	{
		if (msg.t <= (*msg_iter).t) break;
	}
	l.insert(msg_iter,msg);
}

template <class X>
LogicalProcess<X>::~LogicalProcess()
{
//	std::cerr << avail.size() << std::endl;
	fossilCollect(Time::Inf(),NULL);
	// destroy lock
	omp_destroy_lock(&lock);
}

} // end of namespace 

#endif
