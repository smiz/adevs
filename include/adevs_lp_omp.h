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
#include "adevs_time.h"
#include "adevs_message_q.h"
#include "adevs_msg_manager.h"
#include "adevs_abstract_simulator.h"
#include "object_pool.h"
#include "adevs_simulator.h"
#include <omp.h>
#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <queue>
#include <limits.h>
#include <cassert>

namespace adevs
{

/**
 * A logical process is assigned to every atomic model and it simulates
 * that model conservatively. 
 */
template <class X> class LogicalProcess:
	public EventListener<X>
{
	public:
		/**
		 * Constructor builds a logical process without any models
		 * assigned to it.
		 */
		LogicalProcess(int ID, const std::vector<int>& I, const std::vector<int>& E,
				LogicalProcess<X>** all_lps, AbstractSimulator<X>* sim,
				MessageManager<X>* msg_manager);
		/**
		 * Assign a model to this logical process. The model must have a positive
		 * lookahead.
		 */
		void addModel(Devs<X>* model);
		/**
		 * Send a message to the logical process. This will put the message into the
		 * back of the input queue.
		 */
		void sendMessage(Message<X>& msg) { input_q.insert(msg); }
		/**
		 * Get the smallest of the local time of next event. 
		 */
		Time getNextEventTime() { return sim.nextEventTime(); } 
		// Get the process ID
		int getID() const { return ID; }
		/**
		 * Destructor leaves the models intact.
		 */
		~LogicalProcess();
		/// Run the main simulation loop
		void run(double t_stop);
		void outputEvent(Event<X> x, double t)
		{
			psim->notify_output_listeners(x.model,x.value,t);
		}
		void stateChange(Atomic<X>* model, double t)
		{
			psim->notify_state_listeners(model,t);
		}
		void notifyInput(Atomic<X>* model, X& value);
	private:
		// ID of this LP
		const int ID;
		// List of influencees and influencers
		const std::vector<int> E, I;
		// All of the LPs
		LogicalProcess<X>** all_lps;
		// Lookahead for this LP
		double lookahead;
		// Earliest input times 
		std::map<int,Time> eit_map;
		// Input messages to the LP
		MessageQ<X> input_q;
		// Priority queue of messages to process
		std::priority_queue<Message<X> > xq;
		Bag<Event<X> > xb;
		// Smallest of the earliest input times
		Time eit, eot, tSelf;
		// Abstract simulator for notifying listeners
		AbstractSimulator<X>* psim;
		// For managing inter-lp messages
		MessageManager<X>* msg_manager;
		// Simulator for computing state transitions and outputs
		Simulator<X> sim;
		void sendEOT(Time tStop);
		void processInputMessages();
		void addToSimulator(Devs<X>* model);
		void cleanup_xb();
};

template <typename X>
LogicalProcess<X>::LogicalProcess(int ID, const std::vector<int>& I, const std::vector<int>& E,
		LogicalProcess<X>** all_lps, AbstractSimulator<X>* psim, MessageManager<X>* msg_manager):
	ID(ID),E(E),I(I),all_lps(all_lps),psim(psim),msg_manager(msg_manager),sim(this)
{
	tSelf = eot = eit = Time(0.0,0);
	all_lps[ID] = this;
	lookahead = DBL_MAX;
	for (typename std::vector<int>::const_iterator iter = I.begin();
			iter != I.end(); iter++)
		if (*iter != ID) eit_map[*iter] = Time(0.0,0);
	sim.addEventListener(this);
}

template <typename X>
void LogicalProcess<X>::addModel(Devs<X>* model)
{
	lookahead = std::min(model->lookahead(),lookahead);
	assert(lookahead > 0.0);
	// Add it to the simulator and set the processor
	// assignments for the sub-models
	addToSimulator(model);
}

template <typename X>
void LogicalProcess<X>::addToSimulator(Devs<X>* model)
{
	// Assign the model to this LP
	model->setProc(ID);
	Atomic<X>* a = model->typeIsAtomic();
	if (a != NULL)
	{
		sim.addModel(a);
		tSelf.t = sim.nextEventTime();
	}
	else
	{
		Set<Devs<X>*> components;
		model->typeIsNetwork()->getComponents(components);
		typename Set<Devs<X>*>::iterator iter = components.begin();
		for (; iter != components.end(); iter++)
		{
			addToSimulator(*iter);
		}
	}
}

template <typename X>
void LogicalProcess<X>::notifyInput(Atomic<X>* model, X& value)
{
	assert(model->getProc() != ID);
	// Send the event to the proper LP
	Message<X> msg(msg_manager->clone(value));
	msg.t = tSelf;
	msg.src = this;
	msg.target = model;
	msg.type = Message<X>::OUTPUT;
	all_lps[model->getProc()]->sendMessage(msg);
}

template <typename X>
void LogicalProcess<X>::sendEOT(Time tStop)
{
	Message<X> msg;
	msg.target = NULL;
	msg.src = this;
	msg.type = Message<X>::EIT;
	msg.t = tStop;
	msg.t.t += lookahead;
	msg.t.c = 0;
	if (tSelf <= msg.t)
	{
		msg.t = tSelf;
		// If we have sent them, then advance the EOT by one discrete step
		if (tSelf == eit) msg.t.c += 1;
	}
	assert(msg.t.c >= 0);
	// Our next event time can shrink, but EOT is a strictly increasing quantity
	if (msg.t <= eot) return;
	else eot = msg.t;
	for (std::vector<int>::const_iterator iter = E.begin();
			iter != E.end(); iter++)
		if (*iter != ID) all_lps[(*iter)]->sendMessage(msg);
}

template <typename X>
void LogicalProcess<X>::processInputMessages()
{
	while (!input_q.empty())
	{
		Message<X> msg(input_q.remove());
		eit_map[msg.src->getID()] = msg.t;
		if (msg.type == Message<X>::OUTPUT)
			xq.push(msg);
	}
	eit = Time::Inf();
	for (std::map<int,Time>::iterator iter = eit_map.begin();
			iter != eit_map.end(); iter++)	
		eit = std::min((*iter).second,eit);
}

template <typename X>
void LogicalProcess<X>::run(double t_stop)
{
	while (true)
	{
		bool tstop_reached = false;
		// Make sure we stop at t_stop
		Time tStop(eit);
		if (Time::Inf() <= tStop || tStop.t > t_stop)
		{
			tStop = Time(t_stop,UINT_MAX);
			tstop_reached = true;
		}
		while (tSelf <= tStop || (!xq.empty() && xq.top().t <= tStop))
		{
			// Find the time of the next event
			Time tN(tSelf);
			if (!xq.empty() && xq.top().t < tN) tN = xq.top().t;
			if (tSelf == tN) sim.computeNextOutput();
			// If this is at the EIT, the we don't have the input at tN
			// yet and must wait to compute the next state of the model
			if (tN == eit) { assert(!tstop_reached); break; }
			// Find and inject pending input
			while (!xq.empty() && xq.top().t <= tN)
			{
				Message<X> msg(xq.top());
				xq.pop();
				assert(msg.target->getProc() == ID);
				Event<X> input_event(msg.target,msg.value);
				xb.insert(input_event);
			}
			assert(tN.t < DBL_MAX);
			sim.computeNextState(xb,tN.t);
			cleanup_xb();
			// What is our next internal event time?
			tSelf = tN; // last event
			if (tSelf.t < sim.nextEventTime()) // next event
			{
				tSelf.t = sim.nextEventTime();
				tSelf.c = 0;
			}
			else tSelf.c++;
		}
		// Send our earliest output time estimate
		sendEOT(tStop);
		if (tstop_reached) return;
		// Get any available input
		processInputMessages();
	}
}

template <class X>
void LogicalProcess<X>::cleanup_xb()
{
	typename Bag<Event<X> >::iterator iter = xb.begin();
	for (; iter != xb.end(); iter++)
		msg_manager->destroy((*iter).value);
	xb.clear();
}

template <class X>
LogicalProcess<X>::~LogicalProcess()
{
	while (!xq.empty())
	{
		Message<X> msg(xq.top());
		xq.pop();
		Event<X> input_event(msg.target,msg.value);
		xb.insert(input_event);
	}
	cleanup_xb();
}

} // end of namespace 

