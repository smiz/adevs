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
#include <omp.h>
#include <vector>
#include <list>
#include <map>
#include <queue>
#include <limits.h>
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

template <typename X> struct Junk
{
	Time t;
	Atomic<X>* owner;
	Bag<X>* y;
};

template <typename X> struct Message
{
	typedef enum { OUTPUT, EIT, LET } msg_type_t;
	Time t;
	LogicalProcess<X> *src;
	Atomic<X>* target;
	X value;
	msg_type_t type;
	// Sort by timestamp, smallest timestamp first in the STL priority_queue
	bool operator<(const Message<X>& other) const
	{
		return other.t < t;
	}
};

template <class X> class MessageQ
{
	public:
		MessageQ()
		{
			omp_init_lock(&lock);
			qsize = 0;
		}
		void insert(const Message<X>& msg)
		{
			omp_set_lock(&lock);
			q.push_back(msg);
			qsize++;
			omp_unset_lock(&lock);
		}
		Message<X> remove()
		{
			Message<X> msg;
			while (qsize == 0);
			omp_set_lock(&lock);
			msg = q.front();
			q.pop_front();
			qsize--;
			omp_unset_lock(&lock);
			return msg;
		}
		~MessageQ()
		{
			omp_destroy_lock(&lock);
		}
	private:
		omp_lock_t lock;
		std::list<Message<X> > q;
		volatile int qsize;
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
		LogicalProcess(int ID, const std::vector<int>& I, const std::vector<int>& E,
				LogicalProcess<X>** all_lps, AbstractSimulator<X>* sim);
		/**
		 * Assign a model to this logical process.
		 */
		void addModel(Atomic<X>* model);
		/**
		 * Send a message to the logical process. This will put the message into the
		 * back of the input queue.
		 */
		void sendMessage(Message<X>& msg) { input_q.insert(msg); }
		/**
		 * Get the smallest of the local time of next event. 
		 */
		Time getNextEventTime() { return sched.minPriority(); } 
		// Get the process ID
		int getID() const { return ID; }
		/**
		 * Destructor leaves the models intact.
		 */
		~LogicalProcess();
		// Run the main simulation loop
		void run(double t_stop);
	private:
		/// ID of this LP
		const int ID;
		// List of influencees and influencers
		const std::vector<int> E, I;
		// All of the LPs
		LogicalProcess<X>** all_lps;
		// Lookahead for this LP
		double lookahead;
		// Earliest input times and last event timers
		std::map<int,Time> eit, let;
		// Input messages to the LP
		MessageQ<X> input_q;
		// Priority queue of messages to process
		std::priority_queue<Message<X> > xq;
		// Time order lists of messages inter-lp messages that need to be deleted
		std::list<Junk<X> > inter_lp_msgs;
		// This flag is set to true by the route method if the message
		// goes to another LP
		bool inter_lp;
		// Smallest of the earliest input times
		Time min_eit, prev_eot;
		// Bags of imminent and active models.
		Bag<Atomic<X>*> imm, activated;
		/// The event schedule
		Schedule<X,Time> sched;
		/// Pools of preallocated, commonly used objects
		object_pool<Bag<X> > io_pool;
		object_pool<Bag<Event<X> > > recv_pool;
		// Abstract simulator for notifying listeners
		AbstractSimulator<X>* sim;
		// Send the EOT
		void sendEOT();
		// Send the last event time to influencers
		void sendLET(Time tL);
		// Find the smallest of the EIT
		void updateEIT();
		// Route events using the Network models' route methods
		void route(Network<X>* parent, Devs<X>* src, X& x);
		// Compute state transition of an atomic model and reschedule it
		void exec_event(Atomic<X>* model, bool internal, Time t);
		// Inject an input into an atomic model
		void inject_event(Atomic<X>* model, X& value);
		// Build the imminent set and route the output
		void computeOutput();
		// Delete inter LP messages that are no longer in use
		void cleanupInterLPMsgs();
};

template <typename X>
LogicalProcess<X>::LogicalProcess(int ID, const std::vector<int>& I, const std::vector<int>& E,
		LogicalProcess<X>** all_lps, AbstractSimulator<X>* sim):
	ID(ID),E(E),I(I),all_lps(all_lps),sim(sim)
{
	prev_eot = Time(0.0,0);
	all_lps[ID] = this;
	lookahead = DBL_MAX;
	for (typename std::vector<int>::const_iterator iter = I.begin();
			iter != I.end(); iter++)
		if (*iter != ID) eit[*iter] = Time(0.0,0);
	for (typename std::vector<int>::const_iterator iter = E.begin();
			iter != E.end(); iter++)
		if (*iter != ID) let[*iter] = Time(0.0,0);
	updateEIT();
}

template <typename X>
void LogicalProcess<X>::addModel(Atomic<X>* model)
{
	lookahead = std::min(model->lookahead(),lookahead);
	assert(lookahead > 0.0);
	// Assign the model to this LP
	model->par_info.lp = this;
	// Put it into the schedule
	double dt = model->ta();
	if (dt < DBL_MAX) sched.schedule(model,Time(dt,0));
}

template <typename X>
void LogicalProcess<X>::cleanupInterLPMsgs()
{
	Time min_let = Time::Inf();
	for (std::map<int,Time>::iterator iter = let.begin();
			iter != let.end(); iter++)	
		min_let = std::min((*iter).second,min_let);
	while (!inter_lp_msgs.empty() && inter_lp_msgs.front().t <= min_let)
	{
		Atomic<X>* model = inter_lp_msgs.front().owner;
		Bag<X>* garbage = inter_lp_msgs.front().y;
		inter_lp_msgs.pop_front();
		model->gc_output(*garbage);
		garbage->clear();
		io_pool.destroy_obj(garbage);
	}
}

template <typename X>
void LogicalProcess<X>::sendLET(Time tL)
{
	Message<X> msg;
	msg.target = NULL;
	msg.src = this;
	msg.type = Message<X>::LET;
	msg.t = tL;
	for (std::vector<int>::const_iterator iter = I.begin();
			iter != I.end(); iter++)
		if (*iter != ID) all_lps[(*iter)]->sendMessage(msg);
}

template <typename X>
void LogicalProcess<X>::sendEOT()
{
	Message<X> msg;
	msg.target = NULL;
	msg.src = this;
	msg.type = Message<X>::EIT;
	msg.t = min_eit;
	msg.t.t += lookahead;
	msg.t.c = 0;
	if (sched.minPriority() <= msg.t)
	{
		msg.t = sched.minPriority();
		// If we have sent them, then advance the EOT by one discrete step
		if (!imm.empty()) msg.t.c += 1;
	}
	if (prev_eot == msg.t) return;
	else prev_eot = msg.t;
	for (std::vector<int>::const_iterator iter = E.begin();
			iter != E.end(); iter++)
		if (*iter != ID) all_lps[(*iter)]->sendMessage(msg);
}

template <typename X>
void LogicalProcess<X>::updateEIT()
{
	min_eit = Time::Inf();
	for (std::map<int,Time>::iterator iter = eit.begin();
			iter != eit.end(); iter++)	
		min_eit = std::min((*iter).second,min_eit);
}

template <typename X>
void LogicalProcess<X>::computeOutput()
{
	// Don't do anything if the output is up to date
	if (!imm.empty()) return;
	sched.getImminent(imm);
	// Calculate and route the output for the imminent models
	for (typename Bag<Atomic<X>*>::iterator imm_iter = imm.begin(); 
		imm_iter != imm.end(); imm_iter++)
	{
		Atomic<X>* model = *imm_iter;
		model->y = io_pool.make_obj();
		model->output_func(*(model->y));
		inter_lp = false;
		// Route each event in y
		for (typename Bag<X>::iterator y_iter = model->y->begin(); 
			y_iter != model->y->end(); y_iter++)
		{
			// Send messages to local models and other LPs
			route(model->getParent(),model,*y_iter);
		}
		if (inter_lp)
		{
			Junk<X> junk_msg;
			junk_msg.owner = model;
			junk_msg.t = sched.minPriority();
			junk_msg.y = model->y;
			inter_lp_msgs.push_back(junk_msg);
			model->y = NULL;
		}
	}
}

template <typename X>
void LogicalProcess<X>::run(double t_stop)
{
	Time tL = Time(0.0,0);
	bool tstop_reached = false;
	while (true)
	{
		bool send_let = false;
		// Make sure we stop at t_stop
		Time tStop = min_eit;
		if (tStop == Time::Inf() || tStop > Time(t_stop,UINT_MAX))
		{
			tStop = Time(t_stop,UINT_MAX);
			tstop_reached = true;
		}
		// Simulate until that time
		while ((!sched.empty() && sched.minPriority() <= tStop) ||
				(!xq.empty() && xq.top().t <= tStop))
		{
			// Find the time of the next event
			Time tN = sched.minPriority();
			if (!xq.empty() && xq.top().t < tN) tN = xq.top().t;
			assert(imm.empty() || tN == sched.minPriority());
			if (tN == sched.minPriority()) computeOutput();
			// If this is at the EIT, the we don't have the input at tN
			// yet and must wait to compute the next state of the model
			if (tN == min_eit) break;
			// Find and inject pending input
			while (!xq.empty() && xq.top().t <= tN)
			{
				Message<X> msg = xq.top();
				xq.pop();
				inject_event(msg.target,msg.value);
			}
			// Compute new states for the models
			for (typename Bag<Atomic<X>*>::iterator imm_iter = imm.begin(); 
					imm_iter != imm.end(); imm_iter++)
				exec_event(*imm_iter,true,tN);
			for (typename Bag<Atomic<X>*>::iterator imm_iter = activated.begin(); 
					imm_iter != activated.end(); imm_iter++)
				exec_event(*imm_iter,false,tN);
			// Clean up intra lp output
			for (typename Bag<Atomic<X>*>::iterator imm_iter = imm.begin(); 
				imm_iter != imm.end(); imm_iter++)
			{
				if ((*imm_iter)->y != NULL)
				{
					(*imm_iter)->gc_output(*((*imm_iter)->y));
					(*imm_iter)->y->clear();
					io_pool.destroy_obj((*imm_iter)->y);
					(*imm_iter)->y = NULL;
				}
			}
			// Clear the active model lists
			imm.clear();
			activated.clear();
			// Should we send a LET message when done?
			send_let = true;
			tL = tN;
		}
		if (send_let) sendLET(tL);
		// Send our earliest output time estimate
		sendEOT();
		if (tstop_reached) return;
		// Get a message from the input queue
		Message<X> msg = input_q.remove();
		// If it is a NULL message, then update the EIT
		if (msg.type == Message<X>::EIT)
		{
			eit[msg.src->getID()] = msg.t;
			updateEIT();
		}
		// If it is a LET message, then update the earliest input time and
		// clean up if we can
		else if (msg.type == Message<X>::LET)
		{
			let[msg.src->getID()] = msg.t;
			cleanupInterLPMsgs();
		}
		// Otherwise put it into the input queue
		else xq.push(msg);
	}
}

template <typename X>
void LogicalProcess<X>::exec_event(Atomic<X>* model, bool internal, Time t)
{
	if (model->x == NULL)
	{
		model->delta_int();
	}
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
	sim->notify_state_listeners(model,t.t);
}

template <typename X>
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

template <typename X>
void LogicalProcess<X>::route(Network<X>* parent, Devs<X>* src, X& x)
{
	// No one to do the routing, so return
	if (parent == NULL) return;
	// If this is not an input to a coupled model
	if (parent != src) sim->notify_output_listeners(src,x,sched.minPriority().t);
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
			if (amodel->par_info.lp == this)
				inject_event(amodel,(*recv_iter).value);
			// Atomic model at another LP
			else 
			{
				Message<X> msg;
				msg.src = this;
				msg.t = sched.minPriority();
				msg.target = amodel;
				msg.value = (*recv_iter).value;
				msg.type = Message<X>::OUTPUT;
				amodel->par_info.lp->sendMessage(msg);
				inter_lp = true;
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
LogicalProcess<X>::~LogicalProcess()
{
	// Delete any inter lp messages that are still hanging around
	let.clear();
	cleanupInterLPMsgs();
	// Cleanup input and output bags that are still lingering
	for (typename Bag<Atomic<X>*>::iterator imm_iter = imm.begin(); 
		imm_iter != imm.end(); imm_iter++)
	{
		if ((*imm_iter)->x != NULL) io_pool.destroy_obj((*imm_iter)->x);
		if ((*imm_iter)->y != NULL)
		{
			(*imm_iter)->gc_output(*((*imm_iter)->y));
			io_pool.destroy_obj((*imm_iter)->y);
		}
	}
	// Cleanup input and output bags that are still lingering
	for (typename Bag<Atomic<X>*>::iterator imm_iter = activated.begin(); 
		imm_iter != activated.end(); imm_iter++)
	{
		if ((*imm_iter)->x != NULL) io_pool.destroy_obj((*imm_iter)->x);
	}
}

} // end of namespace 

#endif
