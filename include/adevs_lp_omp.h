/**
 * Copyright (c) 2013, James Nutaro
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of the FreeBSD Project.
 *
 * Bugs, comments, and questions can be sent to nutaro@gmail.com
 */
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
#include <limits>
#include <cassert>

namespace adevs
{

/**
 * A logical process is assigned to every atomic model and it simulates
 * that model conservatively. 
 */
template <class X, class T = double> class LogicalProcess:
	public EventListener<X,T>
{
	public:
		/**
		 * Constructor builds a logical process without any models
		 * assigned to it.
		 */
		LogicalProcess(int ID, const std::vector<int>& I,
			const std::vector<int>& E, LogicalProcess<X,T>** all_lps, 
			AbstractSimulator<X,T>* sim, MessageManager<X>* msg_manager);
		/**
		 * Assign a model to this logical process. The model must have a
		 * positive lookahead.
		 */
		void addModel(Devs<X,T>* model);
		/**
		 * Send a message to the logical process. This will put the 
		 * message into the back of the input queue.
		 */
		void sendMessage(Message<X,T>& msg) { input_q.insert(msg); }
		/**
		 * Get the smallest of the local time of next event. 
		 */
		Time<T> getNextEventTime() { return sim.nextEventTime(); } 
		// Get the process ID
		int getID() const { return ID; }
		/**
		 * Destructor leaves the models intact.
		 */
		~LogicalProcess();
		/// Run the main simulation loop
		void run(T t_stop);
		void outputEvent(Event<X,T> x, T t)
		{
			if (looking_ahead) return;
			psim->notify_output_listeners(x.model,x.value,t);
		}
		void stateChange(Atomic<X,T>* model, T t)
		{
			if (looking_ahead) return;
			psim->notify_state_listeners(model,t);
		}
		void notifyInput(Atomic<X,T>* model, X& value);
	private:
		// ID of this LP
		const int ID;
		// List of influencees and influencers
		const std::vector<int> E, I;
		// All of the LPs
		LogicalProcess<X,T>** all_lps;
		// Lookahead for this LP
		T lookahead;
		bool looking_ahead;
		// Earliest input times 
		std::map<int,Time<T> > eit_map;
		// Input messages to the LP
		MessageQ<X,T> input_q;
		// Priority queue of messages to process
		std::priority_queue<Message<X,T> > xq;
		Bag<Event<X,T> > xb;
		// Smallest of the earliest input times
		Time<T> eit, eot, tNow, tOut, tL;
		// Abstract simulator for notifying listeners
		AbstractSimulator<X,T>* psim;
		// For managing inter-lp messages
		MessageManager<X>* msg_manager;
		// Simulator for computing state transitions and outputs
		Simulator<X,T> sim;
		void advanceOutput();
		void sendEOT(Time<T> tNext);
		// Returns true if it reaches t_stop
		void advanceState(T t_stop);
		void processInputMessages();
		void addToSimulator(Devs<X,T>* model);
		Time<T> tNextEvent(Time<T> t);
		void cleanup_xb();
};

template <typename X, class T>
LogicalProcess<X,T>::LogicalProcess(int ID, const std::vector<int>& I, 
	const std::vector<int>& E, LogicalProcess<X,T>** all_lps,
	AbstractSimulator<X,T>* psim, MessageManager<X>* msg_manager):
	ID(ID),E(E),I(I),all_lps(all_lps),psim(psim),
	msg_manager(msg_manager),sim(this)
{
	tL = tOut = tNow = eot = eit = Time<T>(0,0);
	all_lps[ID] = this;
	lookahead = adevs_inf<T>();
	looking_ahead = false;
	for (typename std::vector<int>::const_iterator iter = I.begin();
			iter != I.end(); iter++)
		if (*iter != ID) eit_map[*iter] = Time<T>(0,0);
	sim.addEventListener(this);
}

template <typename X, class T>
void LogicalProcess<X,T>::addModel(Devs<X,T>* model)
{
	lookahead = std::min(model->lookahead(),lookahead);
	assert(lookahead > adevs_zero<T>());
	// Add it to the simulator and set the processor
	// assignments for the sub-models
	addToSimulator(model);
}

template <typename X, class T>
void LogicalProcess<X,T>::addToSimulator(Devs<X,T>* model)
{
	// Assign the model to this LP
	model->setProc(ID);
	Atomic<X,T>* a = model->typeIsAtomic();
	if (a != NULL)
	{
		sim.addModel(a);
	}
	else
	{
		Set<Devs<X,T>*> components;
		model->typeIsNetwork()->getComponents(components);
		typename Set<Devs<X,T>*>::iterator iter = components.begin();
		for (; iter != components.end(); iter++)
		{
			addToSimulator(*iter);
		}
	}
}

template <typename X, class T>
void LogicalProcess<X,T>::notifyInput(Atomic<X,T>* model, X& value)
{
	// Don't send messages that have already been sent
	if (tNow <= tOut) return;
	assert(model->getProc() != ID);
	// Send the event to the proper LP
	Message<X,T> msg(msg_manager->clone(value));
	msg.t = tNow;
	msg.src = this;
	msg.target = model;
	msg.type = Message<X,T>::OUTPUT;
	all_lps[model->getProc()]->sendMessage(msg);
}

template <typename X, class T>
Time<T> LogicalProcess<X,T>::tNextEvent(Time<T> tlast)
{
	if (tlast.t < sim.nextEventTime())
	{
		tlast.t = sim.nextEventTime();
		tlast.c = 0;
	}
	else tlast.c++;
	return tlast;
}

template <typename X, class T>
void LogicalProcess<X,T>::advanceState(T t_stop)
{
	// Make sure we stop at t_stop
	Time<T> tStop(eit);
	if (Time<T>::Inf() <= tStop || tStop.t > t_stop)
	{
		tStop = Time<T>(t_stop,numeric_limits<unsigned int>::max());
	}
	// Advance the state to eit or tStop
	while (true)
	{
		Time<T> tSelf(tNextEvent(tL));
		// Find the time of the next event
		if (!xq.empty() && xq.top().t < tSelf) tNow = xq.top().t;
		else tNow = tSelf;
		// Are we done?
		if (tNow.t == adevs_inf<T>() || tStop < tNow ) return; 
		// Send output if this is an internal or confluent event
		if (tNow == tSelf) sim.computeNextOutput();
		// Advance the output window if the state has caught up
		if (tOut <= tNow) tOut = tNow; 
		// If this is at EIT, then we don't have the input at tN
		// yet and must wait to compute the next state of the model
		if (tNow == eit) return; 
		// Find and inject pending input
		while (!xq.empty() && xq.top().t <= tNow)
		{
			Message<X,T> msg(xq.top());
			xq.pop();
			assert(msg.target->getProc() == ID);
			Event<X,T> input_event(msg.target,msg.value);
			xb.insert(input_event);
		}
		// Compute the next state
		assert(tNow.t < adevs_inf<T>());
		sim.computeNextState(xb,tNow.t);
		cleanup_xb();
		// Remember the time of our last event
		tL = tNow; 
	}
}

template <typename X, class T>
void LogicalProcess<X,T>::advanceOutput()
{
	// This is the time for the new output and state
	tNow = tNextEvent(tL);
	// Project the output as far into the future 
	// as possible
	if (eit.t < adevs_inf<T>() && lookahead < adevs_inf<T>())
	{
		looking_ahead = true;
		sim.beginLookahead();
		// Try to advance the output trajectory
		while (tNow.t < adevs_inf<T>() && tNow < eit + lookahead)
		{
			bool ok = true;
			try
			{
				assert(tNow.t == sim.nextEventTime());
				/**
				 * This computes the next output and might
				 * compute the next state.
				 */ 
				sim.lookNextEvent();
			}
			catch (lookahead_impossible_exception)
			{
				/**
				 * Failed to compute the next state.
				 */
				ok = false;
			}
			if (tOut <= tNow) tOut = tNow;
			// If we can and there is nothing more useful to do,
			// move on to the next autonomous event
			if (ok) tNow = tNextEvent(tNow);
			else break;
		}
		sim.endLookahead();
		assert(tNextEvent(tL).t == sim.nextEventTime());
		looking_ahead = false;
	} 
	sendEOT(tNow);
}

template <typename X, class T>
void LogicalProcess<X,T>::sendEOT(Time<T> tNext)
{
	// Send a new value for the earliest output time
	Time<T> newEot(eit+lookahead);
	if (tNext < newEot) newEot = tNext;
	if (newEot == eit) newEot.c++;
	// If this new EOT value is greater than our previous EOT
	// value then sent it to the downstream LPs
	if (eot < newEot)
	{
		eot = newEot;
		Message<X,T> msg;
		msg.target = NULL;
		msg.src = this;
		msg.type = Message<X,T>::EIT;
		msg.t = eot; 
		for (std::vector<int>::const_iterator iter = E.begin();
			iter != E.end(); iter++)
			if (*iter != ID) all_lps[(*iter)]->sendMessage(msg);
	}
}

template <typename X, class T>
void LogicalProcess<X,T>::processInputMessages()
{
	while (!input_q.empty())
	{
		Message<X,T> msg(input_q.remove());
		eit_map[msg.src->getID()] = msg.t;
		if (msg.type == Message<X,T>::OUTPUT)
			xq.push(msg);
	}
	eit = Time<T>::Inf();
    typename std::map<int,Time<T> >::iterator iter;
	for (iter = eit_map.begin();
			iter != eit_map.end(); iter++)	
		eit = std::min((*iter).second,eit);
}

template <typename X, class T>
void LogicalProcess<X,T>::run(T t_stop)
{
	bool try_again = true;
	// Run until advanceState reaches the stopping time
	while (
		eit.t <= t_stop ||
		eot.t <= t_stop ||
		tNextEvent(tL).t <= t_stop ||
		(!xq.empty() && xq.top().t.t <= t_stop)
	)
	{

		if (try_again)
		{
			advanceState(t_stop);
			advanceOutput();
		}
		Time<T> eit_now(eit);
		processInputMessages();
		try_again = eit_now < eit; 
	}
}

template <class X, class T>
void LogicalProcess<X,T>::cleanup_xb()
{
	typename Bag<Event<X,T> >::iterator iter = xb.begin();
	for (; iter != xb.end(); iter++)
		msg_manager->destroy((*iter).value);
	xb.clear();
}

template <class X, class T>
LogicalProcess<X,T>::~LogicalProcess()
{
	while (!xq.empty())
	{
		Message<X,T> msg(xq.top());
		xq.pop();
		Event<X,T> input_event(msg.target,msg.value);
		xb.insert(input_event);
	}
	cleanup_xb();
}

} // end of namespace 

