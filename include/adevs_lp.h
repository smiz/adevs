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
#include "adevs_time.h"
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

// Enumeration of simulation message types
typedef enum
{
	RB, // rollback message
	IO // output (input) message
} MessageType;

// A simulation message
template <class X> struct Message
{
	// The message timestamp
	Time t;
	// The logical process that generated the message
	LogicalProcess<X>* src;
	// The value of the message
	X value;
	// The type of message
	MessageType type;
};

// A state checkpoint
struct CheckPoint
{
	// Time stamp
	Time t;
	// Pointer to the saved state
	void* data;
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
		 * The constructor assigns an atomic model to the logical process.
		 * The active list is a shared list that is used to keep track of
		 * which lps are activated by message delivery in each simulation
		 * round.
		 */
		LogicalProcess(Atomic<X>* model, std::vector<LogicalProcess*>* active_list);
		/** 
		 * Process all of the events in the input list.
		 */
		void processInput();
		/**
		 * Optimistically execute the output and state transition functions.
		 */
		void execEvents();
		/**
		 * Do fossil collection and report correct states and outputs.
		 */
		void fossilCollect(Time gvt, const Bag<EventListener<X>*>& listeners);
		/**
		 * Get the model assigned to this lp.
		 */
		Atomic<X>* getModel() { return model; }
		/**
		 * Send a message to the logical process. This will put the message into the
		 * back of the input queue.
		 */
		void sendMessage(Message<X>& m);
		/**
		 * Get the smallest of the local time of next event and first input message
		 */
		Time getNextEventTime() const
		{
			Time result = Time::Inf();
			if (time_advance < DBL_MAX)
				result = tL + Time(time_advance,0);
			if (!avail.empty() && avail.back().t < result)
				result = avail.back().t;
			if (!input.empty() && tMinInput < result)
				result = tMinInput;
			if (rb_time < result)
				result = rb_time;
			return result;
		}
		/**
		 * Get the event time for the current system state.
		 */
		Time getLocalStateTime() const { return tL; }
		/**
		 * Set or clear the active flag.
		 */
		void setActive(bool flag) { model->active = flag; }
		/**
		 * Has this lp been activated in this round.
		 */
		bool isActive() const { return model->active; }
		/**
		 * Destructor leaves the atomic model intact.
		 */
		~LogicalProcess();
	private:
		// Time of the last committed state
		Time lastCommit;
		// Time advance in the present state
		double time_advance;
		// The time of the last last event
		Time tL;
		// List of input messages
		std::list<Message<X> > input;
		// lock variable
		omp_lock_t lock;
		/**
		 * All of these lists are sorted by time stamp
		 * with the earliest time stamp at the BACK.
		 */
		// Time ordered list of messages that are available for processing
		std::list<Message<X> > avail;
		// Time ordered list of messages that have been processed
		std::list<Message<X> > used;
		/**
		 * All of these lists are sorted by time stamp
		 * with the earliest time stamp at the FRONT.
		 */
		// Time ordered list of good output messages
		std::list<Message<X> > output;
		// Time ordered list of discarded output messages
		std::list<Message<X> > discard;
		// Time ordered list of checkpoints
		std::list<CheckPoint> chk_pt;
		// Set of lps that I have sent a message to
		std::set<LogicalProcess<X>*> recipients;
		// What is the timestamp of the pending rollback 
		Time rb_time;
		// Smallest input timestamp
		Time tMinInput;
		// Temporary pointer to an active list that is provided by the simulator
		std::vector<LogicalProcess<X>*>* active_list;
		// The atomic model assigned to this logical process
		Atomic<X>* model;
		// Input and output bag for the model. Always clear this before using it.
		Bag<X> io_bag;
		// Insert a message into a timestamp ordered list
		void insert_message(std::list<Message<X> >& l, Message<X>& msg, bool back_to_front = false);
		// Route events using the Network models' route methods
		void route(Network<X>* parent, Devs<X>* src, X& x);
		// Report correct states
		void listenerCallback(const Bag<EventListener<X>*>& listeners, Time t, void* state);
		// Report correct output
		void listenerCallback(const Bag<EventListener<X>*>& listeners, Message<X>& msg);
};

template <class X>
LogicalProcess<X>::LogicalProcess(Atomic<X>* model, std::vector<LogicalProcess<X>*>* active_list):
	rb_time(Time::Inf()),
	active_list(active_list),
	model(model)
{
	// Set the local time of next event
	time_advance = model->ta();
	// The model is intially inactive
	model->active = false;
	// initialize lock
	omp_init_lock(&lock);	
}

template <class X>
void LogicalProcess<X>::listenerCallback(const Bag<EventListener<X>*>& listeners, Time t, void* state)
{
	typename Bag<EventListener<X>*>::const_iterator iter = listeners.begin();
	for (; iter != listeners.end(); iter++)
		(*iter)->stateChange(model,t.t,state);
}

template <class X>
void LogicalProcess<X>::listenerCallback(const Bag<EventListener<X>*>& listeners, Message<X>& msg)
{
	Event<X> event(model,msg.value);
	typename Bag<EventListener<X>*>::const_iterator iter = listeners.begin();
	for (; iter != listeners.end(); iter++)
		(*iter)->outputEvent(event,msg.t.t);
}

template <class X>
void LogicalProcess<X>::fossilCollect(Time gvt, const Bag<EventListener<X>*>& listeners)
{
	// Delete old states, but keep one that is less than gvt
	std::list<CheckPoint>::iterator citer = chk_pt.begin(), cnext;
	while (citer != chk_pt.end())
	{
		// Report citer if it is less than gvt
		if ((*citer).t < gvt && (*citer).t > lastCommit)
		{
			lastCommit = (*citer).t;
			listenerCallback(listeners,lastCommit,(*citer).data);
		}
		// Do we have another saved state smaller than gvt?
		cnext = citer;
		cnext++;
		// If so, delete citer
		if (cnext != chk_pt.end() && (*cnext).t < gvt)
		{
			model->gc_state((*citer).data);
			citer = chk_pt.erase(citer);
		}
		// Otherwise we are done cleaning up
		else break;
	}
	// Report the current state if that is appropriate
	if (tL < gvt && tL > lastCommit)
	{
		lastCommit = tL;
		listenerCallback(listeners,lastCommit,NULL);
	}
	// Delete old used messages
	while (!used.empty() && used.back().t < gvt)
	{
		used.pop_back();
	}
	// Delete old output
	io_bag.clear();
	while (!discard.empty() && discard.front().t < gvt) 	
	{
		io_bag.insert(discard.front().value);
		discard.pop_front();
	}
	while (!output.empty() && output.front().t < gvt) 
	{
		listenerCallback(listeners,output.front());
		io_bag.insert(output.front().value);
		output.pop_front();
	}
	if (!io_bag.empty()) model->gc_output(io_bag);
}

template <class X>
void LogicalProcess<X>::processInput()
{
	// Process all of the input messages
	for (;;) //(!input.empty())
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
		if (no_input) break;
		// Was a used message actually canceled?
		bool used_msg_cancelled = false;
		// If this is a rollback message then discard messages from the sender.
		// Remember if one of these is a used message in the past.
		if (msg.type == RB)
		{
			// Discard unprocessed messages that are in the future
			typename std::list<Message<X> >::iterator msg_iter = avail.begin();
			while (msg_iter != avail.end())
			{
				if (msg.t <= (*msg_iter).t)
				{
					if ((*msg_iter).src == msg.src) msg_iter = avail.erase(msg_iter);
					else msg_iter++;
				}
				else break;
			}
			// Discard processed messages that are in the past
			msg_iter = used.begin();
			while (msg_iter != used.end())
			{
				if (msg.t <= (*msg_iter).t)
				{
					if ((*msg_iter).src == msg.src)
					{
						msg_iter = used.erase(msg_iter);
						used_msg_cancelled = true;
					}
					else msg_iter++;
				}
				else break;
			}
		}
		// Discard the incorrect outputs. Find the first output after msg.t
		// because that will be the timestamp of our rollback message.
		if (used_msg_cancelled || msg.type != RB)
		{
			Time t_bad = Time::Inf();
			while (!output.empty() && msg.t < output.back().t)
			{
				t_bad = output.back().t;
				insert_message(discard,output.back());
				output.pop_back();
			}
			// Schedule a rollback message
			if (t_bad < rb_time) rb_time = t_bad;
		}
		// If this message is in the past, then perform a rollback
		if ((msg.type != RB && msg.t < tL) || used_msg_cancelled)
		{
			assert(msg.t < tL);
			assert(!chk_pt.empty());
			// Discard incorrect checkpoints
			while (msg.t < chk_pt.back().t)
			{
				model->gc_state(chk_pt.back().data);
				chk_pt.pop_back();
				assert(chk_pt.empty() == false); 
			}
			// Restore the model state 
			tL = chk_pt.back().t;
			model->tL = tL.t; 
			model->restore_state(chk_pt.back().data);
			time_advance = model->ta();
			// Remove the checkpoint
			model->gc_state(chk_pt.back().data);
			chk_pt.pop_back();
			// Make sure the time advance is ok
			if (time_advance < 0.0)
			{
				exception err("Atomic model has a negative time advance",model);
				throw err;
			}
			// Copy used messages back to the available list
			while (!used.empty() && tL <= used.front().t)
			{
				assert(avail.empty() || used.front().t <= avail.back().t);
				avail.push_back(used.front());
				used.pop_front();
			}
		}
		// Add it to the list of available messages
		if (msg.type != RB)	
		{
			insert_message(avail,msg,true);
		}
	} 
	// Done with the input messages. 
}

template <class X>
void LogicalProcess<X>::execEvents()
{
	// Send a pending rollback
	if (rb_time < Time::Inf())
	{
		// Create the rollback message
		Message<X> msg;
		msg.src = this;
		msg.t = rb_time;
		msg.type = RB;
		// Send it to all of the lp's that we've sent a message to
		typename std::set<LogicalProcess<X>*>::iterator lp_iter;
		lp_iter = recipients.begin(); 
		for (; lp_iter != recipients.end(); lp_iter++)
			(*lp_iter)->sendMessage(msg);
		// Cancel the pending rollback
		rb_time = Time::Inf();
	}
	// This is the time of the next internal event
	Time tSelf = Time::Inf();
	if (time_advance < DBL_MAX)
	{
		assert(time_advance >= 0.0);
		tSelf = tL + Time(time_advance,0);
		// This is a pathological case caused by time_advance being very small
		if (tSelf < tL) tSelf = tL;
	}
	Time tN(tSelf);
	// Is there an external input prior to tSelf?
	if (!avail.empty() && avail.back().t < tN)
		tN = avail.back().t;
	// If the next event is at infinity, then there is nothing to do
	if (tN == Time::Inf()) return;
	assert(tL <= tN);
	// If this is an internal event and we haven't already sent this output,
	// then send it
	if (!(tN < tSelf) && (output.empty() || output.back().t < tSelf))
	{
		// Send our output
		Message<X> msg;
		msg.src = this;
		msg.t = tSelf;
		msg.type = IO;
		// Compute the model's output
		io_bag.clear();
		model->output_func(io_bag);
		// Send the the output values
		for (typename Bag<X>::iterator iter = io_bag.begin(); 
			iter != io_bag.end(); iter++)
		{
			assert(output.empty() || output.back().t <= msg.t);
			msg.value = *iter;
			output.push_back(msg);
			route(model->getParent(),model,*iter);
		}
	}
	// Save the current state and compute the next state.
	CheckPoint c;
	c.t = tL;
	c.data = model->save_state();
	chk_pt.push_back(c);
	// Get the inputs
	io_bag.clear();
	while (!avail.empty() && avail.back().t == tN)
	{
		io_bag.insert(avail.back().value);
		assert(used.empty() || avail.back().t >= used.front().t);
		used.push_front(avail.back());
		avail.pop_back();
		assert(avail.empty() || avail.back().t >= used.front().t);
	}
	// Compute the next state
	if (!(tN < tSelf))
	{
		if (io_bag.empty()) model->delta_int();
		else model->delta_conf(io_bag);
	}
	// If this is an external event
	else model->delta_ext(tN.t-tL.t,io_bag);
	// Get the new value of the time advance
	time_advance = model->ta();
	// Actual time for this state
	tL = tN + Time(0.0,1);
	model->tL = tL.t;
	assert(output.empty() || output.back().t <= tL);
}

template <class X>
void LogicalProcess<X>::route(Network<X>* parent, Devs<X>* src, X& x)
{
	// No one to do the routing, so return
	if (parent == NULL) return;
	// Create a bag to collect the receivers
	Bag<Event<X> > recvs;
	// Compute the set of receivers for this value
	parent->route(x,src,recvs);
	// Deliver the event to each of its targets
	Atomic<X>* amodel = NULL;
	typename Bag<Event<X> >::iterator recv_iter = recvs.begin();
	for (; recv_iter != recvs.end(); recv_iter++)
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
			Message<X> m = output.back();
			m.value = (*recv_iter).value;
			amodel->lp->sendMessage(m); 
			recipients.insert(amodel->lp); 
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
}

template <class X>
void LogicalProcess<X>::sendMessage(Message<X>& msg)
{
	bool put_in_active_list = false;
	// Lock the input list
	omp_set_lock(&lock);
	// Set the minimum input time
	if (input.empty()) tMinInput = msg.t;
	else if (msg.t < tMinInput) tMinInput = msg.t;
	// Insert the message
	input.push_back(msg);
	// Check the active status
	if (!(model->active))
	{
		put_in_active_list = model->active = true;
	}
	omp_unset_lock(&lock);
	// Insert the message into the global active list
	if (put_in_active_list)
	{
		#pragma omp critical
		{
			active_list->push_back(this);
		}
	} // end of critical section
}

template <class X>
void LogicalProcess<X>::insert_message(std::list<Message<X> >& l, Message<X>& msg, bool back_to_front)
{
	typename std::list<Message<X> >::iterator msg_iter;
	for (msg_iter = l.begin(); msg_iter != l.end(); msg_iter++)
	{
		if (back_to_front)
		{
			if ((*msg_iter).t < msg.t) break;
		}
		else if (!((*msg_iter).t <= msg.t)) break;
	}
	l.insert(msg_iter,msg);
}

template <class X>
LogicalProcess<X>::~LogicalProcess()
{
	// Delete check points
	while (!chk_pt.empty())
	{
		model->gc_state(chk_pt.front().data);
		chk_pt.pop_front();
	}
	// Clean up remaining output messages
	io_bag.clear();
	while (!output.empty())
	{
		io_bag.insert(output.front().value);
		output.pop_front();
	}
	// Clean up the discarded messages
	while (!discard.empty())
	{
		io_bag.insert(discard.front().value);
		discard.pop_front();
	}
	if (!io_bag.empty()) model->gc_output(io_bag);
	// destroy lock
	omp_destroy_lock(&lock);
}

} // end of namespace 

#endif
