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
typedef enum { SELF, ROLLBACK, INPUT_OUTPUT } MessageType;

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
	// Flag to indicate of this state has been reported
	bool reported;
	// Constructor sets the reported flag to false
	CheckPoint():reported(false){}
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
		 * Optimistically execute the next event at the lp.
		 */
		void execNextEvent();
		/**
		 * Do fossil collection.
		 */
		void fossilCollect(Time gvt);
		/**
		 * Get the list of messages that output messages that are thought
		 * to be good. The messages with timestamps less than or equal to
		 * gvt are guaranteed to be correct.
		 */
		std::list<Message<X> >* getOutput() { return &output; }
		/**
		 * Get the list of save stats that are thought
		 * to be good. The states with timestamps less than or equal to
		 * gvt are guaranteed to be correct.
		 */
		std::list<CheckPoint>* getStates() { return &chk_pt; }
		/**
		 * Get the model assigned to this lp.
		 */
		Atomic<X>* getModel() { return model; }
		/**
		 * Send a message to the logical process. This will put the message into the
		 * back of the input queue.
		 */
		void insertMessage(Message<X> m);
		/**
		 * Get the smallest of the local time of next event and first input message
		 */
		Time getNextEventTime() const
		{
			Time result = lvt;
			if (!input.empty() && input.front().t < result)
				result = input.front().t;
			if (!avail.empty() && avail.front().t < result)
				result = avail.front().t;
			return result;
		}
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
		// The local time of next event
		Time lvt;
		// The time of the last last event
		Time tL;
		// The last rollback time
		Time lr;	
		// List of input messages
		std::list<Message<X> > input;
		/**
		 * All of these lists are sorted by time stamp
		 * with the earliest time stamp at the front.
		 */
		// Time ordered list of messages that are available for processing
		std::list<Message<X> > avail;
		// Time ordered list of messages that have been processed
		std::list<Message<X> > used;
		// Time ordered list of good output messages
		std::list<Message<X> > output;
		// Time ordered list of discarded output messages
		std::list<Message<X> > discard;
		// Time ordered list of checkpoints
		std::list<CheckPoint> chk_pt;
		// List of lps that I have sent a message to
		std::list<LogicalProcess<X>*> recipients;
		// Temporary pointer to an active list that is provided by the simulator
		std::vector<LogicalProcess<X>*>* active_list;
		// The atomic model assigned to this logical process
		Atomic<X>* model;
		// Input and output bag for the model. Always clear this before using it.
		Bag<X> io_bag;
		// Number of available input messages
		int num_input_msgs;
		// Send a output to the set of receiving lps
		void send_output();
		// Route events using the Network models' route methods
		void route(Network<X>* parent, Devs<X>* src, X& x);
};

template <class X>
LogicalProcess<X>::LogicalProcess(Atomic<X>* model, std::vector<LogicalProcess<X>*>* active_list):
	active_list(active_list),
	model(model),
	num_input_msgs(0)
{
	// Save the model's initial state
	CheckPoint c;
	c.data = model->save_state();
	chk_pt.push_back(c);
	// Set the local time of next event
	lvt += Time(model->ta(),1);
	// The model is intially inactive
	model->active = false;
}

template <class X>
void LogicalProcess<X>::fossilCollect(Time gvt)
{
	// Delete old states
	std::list<CheckPoint>::iterator fc_iter = chk_pt.begin();
	while (!chk_pt.empty())
	{
		std::list<CheckPoint>::iterator nfc_iter = fc_iter;
		nfc_iter++;
		if (nfc_iter != chk_pt.end() && (*nfc_iter).t < gvt)
		{
			model->gc_state((*fc_iter).data);
			fc_iter = chk_pt.erase(fc_iter);
		}
		else break;
	}
	// If tL is less than gvt, then get rid of the remaining checkpoint
	if (tL < gvt && !chk_pt.empty())
	{
		assert(chk_pt.size() == 1);
		model->gc_state(chk_pt.back().data);
		chk_pt.pop_back();
	}
	// Delete old used messages
	if (chk_pt.empty()) used.clear();
	while (!used.empty() && used.front().t < chk_pt.front().t)
	{
		used.pop_front();
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
		io_bag.insert(output.front().value);
		output.pop_front();
	}
	if (!io_bag.empty())
		model->gc_output(io_bag);
}

template <class X>
void LogicalProcess<X>::execNextEvent()
{
	// Message list iterator
	typename std::list<Message<X> >::iterator msg_iter;
	/*
	 * Process the next message.
	 */
	if (num_input_msgs > 0)
	{
		// Get the message at the front of the list
		Message<X> msg;
		/**
		 * There is, in effect, only one communication channel
		 * that all of the threads must share. See injectMessage
		 * for the other point where the input list is manipulated.
		 */
		#pragma omp critical
		{
			msg = input.front();
			input.pop_front();
			num_input_msgs--;
		} // end of critical section
		// If this is a roll back message then discard messages from the sender
		if (msg.type == ROLLBACK)
		{
			msg_iter = avail.begin();
			while (msg_iter != avail.end())
			{
				if ((*msg_iter).src == msg.src && (*msg_iter).t >= msg.t)
					msg_iter = avail.erase(msg_iter);
				else 
					msg_iter++;
			}
			msg_iter = used.begin();
			while (msg_iter != used.end())
			{
				if ((*msg_iter).src == msg.src && (*msg_iter).t >= msg.t)
					msg_iter = used.erase(msg_iter);
				else
					msg_iter++;
			}
		}
		// Is a local rollback required?
		if (msg.t <= tL)
		{
			// Save the rollback time
			lr = msg.t;
			// Look for a checkpoint following the message time
			typename std::list<CheckPoint>::iterator c_iter = chk_pt.begin();
			for (; c_iter != chk_pt.end(); c_iter++)
			{
				// Send a rollback message when the earliest one is found
				if ((*c_iter).t > msg.t)
				{
					// Send the rollback
					Message<X> rb_msg;
					rb_msg.src = this;
					rb_msg.type = ROLLBACK;
					rb_msg.t = (*c_iter).t;
					typename std::list<LogicalProcess<X>*>::iterator lp_iter = recipients.begin();
					for (; lp_iter != recipients.end(); lp_iter++)
						(*lp_iter)->insertMessage(rb_msg);
					// Discard incorrect output
					while (!output.empty() && output.back().t >= rb_msg.t)
					{
						// Keep the discard list in sorted order
						for (msg_iter = discard.begin(); msg_iter != discard.end(); msg_iter++)
						{
							if ((*msg_iter).t > output.back().t) break;
						}
						discard.insert(msg_iter,output.back());
						output.pop_back();
					} 
					break;
				}
			}
			// Discard incorrect checkpoints
			while (chk_pt.back().t >= msg.t)
			{
				model->gc_state(chk_pt.back().data);
				chk_pt.pop_back();
				assert(chk_pt.empty() == false);
			}
			// Restore the model state 
			tL = chk_pt.back().t;
			model->tL = tL.t; 
			model->restore_state(chk_pt.back().data);
			double model_ta = model->ta();
			if (model_ta < 0.0)
			{
				exception err("Atomic model has a negative time advance",model);
				throw err;
			}
			lvt = tL + Time(model->ta(),1);
			// Move messages from the used bag to the available bag
			while (!used.empty() && used.back().t > tL)
			{
				if (!avail.empty()) assert(avail.front().t >= used.back().t);
				avail.push_front(used.back());
				used.pop_back();
			}
		} // Done with the rollback
		// If this wasn't a rollback message then add it to the available list
		if (msg.type != ROLLBACK)
		{
			for (msg_iter = avail.begin(); msg_iter != avail.end(); msg_iter++)
			{
				if ((*msg_iter).t > msg.t) break;
			}
			avail.insert(msg_iter,msg);
		}
	} // Done receiving the message
	// If there are not available messagens and lvt is at infinity, then we're done
	if (avail.empty() && !(lvt.t < DBL_MAX))
		return;
	// Send an output event if the next event is a self event or confluent event
	if ((avail.empty() || avail.front().t >= lvt) && lvt > lr)
		send_output();
	// Construct the input bag for the next state transition
	io_bag.clear();
	Time t_input(DBL_MAX,0);
	if (!avail.empty())
	{
		t_input = avail.front().t;
		if (t_input <= lvt)
		{
			while (!avail.empty() && avail.front().t == t_input)
			{
				io_bag.insert(avail.front().value);
				used.push_back(avail.front());
				avail.pop_front();
			}
		}
	}
	// Compute the next state of the model
	if (t_input > lvt) model->delta_int();
	else if (t_input == lvt) model->delta_conf(io_bag);
	else model->delta_ext(t_input.t-tL.t,io_bag);
	// Compute lvt, tL, and save the model state
	CheckPoint c;
	if (t_input < lvt) c.t = t_input;
	else c.t = lvt;
	tL = c.t;
	model->tL = tL.t;
	c.data = model->save_state();
	chk_pt.push_back(c);
	if (model->ta() < DBL_MAX)
		lvt = tL + Time(model->ta(),1);
	else
		lvt = Time(DBL_MAX,0);
}

template <class X>
void LogicalProcess<X>::send_output()
{
	io_bag.clear();
	model->output_func(io_bag);
	for (typename Bag<X>::iterator iter = io_bag.begin(); 
	iter != io_bag.end(); iter++)
	{
		Message<X> m;
		m.src = this;
		m.t = lvt;
		m.value = *iter;
		m.type = INPUT_OUTPUT;
		output.push_back(m);
		route(model->getParent(),model,*iter);
	}
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
			Message<X> m;
			m.src = this;
			m.t = lvt;
			m.value = (*recv_iter).value;
			m.type = INPUT_OUTPUT;
			amodel->lp->insertMessage(m);
			recipients.push_back(amodel->lp);
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
void LogicalProcess<X>::insertMessage(Message<X> m)
{
	/**
	 * Thread communication can not occur in parallel. There is, in effect,
	 * one communication channel that is shared by all of the threads.
	 */
	#pragma omp critical
	{
		input.push_back(m);
		if (!(model->active))
		{
			active_list->push_back(this);
			model->active = true;
		}
		num_input_msgs++;
	} // end of critical section
}

template <class X>
LogicalProcess<X>::~LogicalProcess()
{
	// Delete check points
	for (typename std::list<CheckPoint>::iterator iter = chk_pt.begin();
			iter != chk_pt.end(); iter++)
		model->gc_state((*iter).data); 
	// Clean up remaining output messages
	io_bag.clear();
	for (typename std::list<Message<X> >::iterator iter = output.begin();
			iter != output.end(); iter++)
		io_bag.insert((*iter).value);
	for (typename std::list<Message<X> >::iterator iter = discard.begin();
			iter != discard.end(); iter++)
		io_bag.insert((*iter).value);
	if (!io_bag.empty())
		model->gc_output(io_bag); 
}

} // end of namespace 

#endif
