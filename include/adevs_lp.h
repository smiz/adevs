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
#include "adevs_list.h"
#include <omp.h>
//#include <TAU.h>
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

struct LP_perf_t
{
	unsigned rollbacks, canceled_output, canceled_intra_lp_output,
			 stragglers, patches, destroyed_in_input, msg_count,
			 chk_pt_count;
	LP_perf_t():
		rollbacks(0),
		canceled_output(0),
		canceled_intra_lp_output(0),
		stragglers(0),
		patches(0),
		destroyed_in_input(0),
		msg_count(0),
		chk_pt_count(0)
	{
	}
};

struct gvt_t
{
	double msg_t;
	double lv_t;
};

/*
 * A logical process is assigned to every atomic model and it simulates
 * that model optimistically. The atomic model must support state saving
 * and state restoration.
 */
template <class X> class LogicalProcess
{
	public:
		// A simulation message
		struct Message:
			public ulist<Message>::one_list
		{
			public:
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
		 * Get the GVT data for this LP.
		 */
		double getLVT() 
		{
			double result = getNextEventTime().t;
			omp_set_lock(&lock);
			typename ulist<Message>::iterator iter = input.begin();
			for (; iter != input.end(); iter++)
				if ((*iter)->t.t < result) result = (*iter)->t.t;
			omp_unset_lock(&lock);
			return result;
		}
		/**
		 * Get the smallest time stamp of any message sent in the last cell to execEvents()
		 * or processInput() since the last call the clear min.
		 */
		double getSendMin() { return send_min; }
		void clearSendMin() { send_min = DBL_MAX; }
		/**
		 * Do fossil collection and report correct states and outputs.
		 */
		void fossilCollect(Time gvt, AbstractSimulator<X>* sim);
		/**
		 * Send a message to the logical process. This will put the message into the
		 * back of the input queue. Returns true if the receiver put the message into
		 * its input list, false otherwise.
		 */
		bool sendMessage(Message* m);
		/**
		 * Get the smallest of the local time of next event. For this to be used in a gvt calculation
		 * the input lists need to be flushed by calling processInput for all LPs until they return
		 * false for pendingInput()
		 */
		Time getNextEventTime() 
		{
			Time result = sched.minPriority();
			if (!avail.empty() && avail.front()->t < result)
				result = avail.front()->t;
			return result;
		}
		/// Are there any events left to execute?
		bool hasEvents()
		{
			return !(input.empty() && avail.empty() && sched.empty());
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
		 * How much garbage is lingering about?
		 */
		unsigned int getGarbageCount() const { return garbage_count; }
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
		class CheckPoint:
			public ulist<CheckPoint>::one_list
		{
			public:
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
		adevs::ulist<Message> input;
		// lock variable for the input list
		omp_lock_t lock;
		// Time ordered list of messages that are available for processing
		adevs::ulist<Message> avail;
		// Time ordered list of messages that have been processed
		adevs::ulist<Message> used;
		// Time ordered list of checkpoints
		adevs::ulist<CheckPoint> chk_pt;
		// Time ordered list of good output messages send to other LPs
		adevs::ulist<Message> inter_lp_output;
		// Time ordered list of good output generated in this LP
		adevs::ulist<Message> intra_lp_output;
		// Unordered list of discarded output messages
		adevs::ulist<Message> discard;
		/// Counter for producing message identifiers
		long int ID;
		/// Can route send messages between LPs?
		bool inter_LP_ok;
		double send_min;
		/// Amount of accumulated trash
		unsigned int garbage_count;
		// Free list for messages
		adevs::ulist<Message> free_msg_list;
		// Free list for checkpoints
		adevs::ulist<CheckPoint> free_chk_pt_list;
		// Bags of imminent and active models.
		Bag<Atomic<X>*> imm, activated;
		/// The event schedule
		Schedule<X,Time> sched;
		/// Pools of preallocated, commonly used objects
		object_pool<Bag<X> > io_pool;
		object_pool<Bag<Event<X> > > recv_pool;
		// Insert a message into a timestamp ordered list
		void insert_message(adevs::ulist<Message>& l, Message* msg);
		// Route events using the Network models' route methods
		void route(Network<X>* parent, Devs<X>* src, X& x, bool* inter_lp_msg = NULL);
		// Save state and compute state transition of an atomic model
		void exec_event(Atomic<X>* model, bool internal, Time t);
		// Inject an input into an atomic model
		void inject_event(Atomic<X>* model, X& value);
		// Destroy a message in the list l with the antimessage msg. Returns true
		// if the message was found and destroyed.
		bool anti_message(adevs::ulist<Message>& l, Message* msg);
		// Try to patch the LP without performing a rollback. Returns true if
		// successful, false otherwise. Failure does not change the state of the
		// LP.
		bool patch(Message* msg);
		// Get a message from the free list
		Message* alloc_msg()
		{
			Message* m = free_msg_list.front();
			if (m == NULL)
			{
				perf_data.msg_count++;
				m = new Message;
			}
			else free_msg_list.pop_front();
			return m;
		}
		CheckPoint* alloc_chk_pt()
		{
			CheckPoint* c = free_chk_pt_list.front();
			if (c == NULL)
			{
				perf_data.chk_pt_count++;
				c = new CheckPoint;
			}
			else free_chk_pt_list.pop_front();
			return c;
		}
};

template <class X>
LogicalProcess<X>::LogicalProcess()
{
	send_min = DBL_MAX;
	garbage_count = 0;
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
//	TAU_PROFILE("fossilCollect","void (Time,AbstractSimulator<X>*)",TAU_DEFAULT);
	// Report and delete old states
	typename adevs::ulist<CheckPoint>::iterator chk_pt_iter = chk_pt.begin();
	while (chk_pt_iter != chk_pt.end())
	{
		Atomic<X>* model = (*chk_pt_iter)->model;
		void* data = (*chk_pt_iter)->data;
		Time ti = (*chk_pt_iter)->ti;
		Time tf = (*chk_pt_iter)->tf;
		// Report states that are good
		if (sim != NULL)
		{
			if (model->tL < gvt && tCommit <= model->tL && model->active == false)
			{
				model->active = true;
				activated.insert(model);
			}
			if (ti < gvt && tCommit <= ti)
				sim->notify_state_listeners(model,ti.t,data); 
		} 
		// Delete states that are no longer needed
		if (tf < gvt)
		{
			model->gc_state(data);
			chk_pt_iter = chk_pt.erase(chk_pt_iter,&free_chk_pt_list);
			garbage_count--;
		}
		else chk_pt_iter++;
	}
	// Report the active models
	typename Bag<Atomic<X>*>::iterator active_iter = activated.begin();
	for (; active_iter != activated.end(); active_iter++)
	{
		sim->notify_state_listeners(*active_iter,(*active_iter)->tL.t);
		(*active_iter)->active = false;
	} 
	activated.clear();
	// Remember the last timestamp that we reported
	if (tCommit < gvt) tCommit = gvt;
	// Delete old used messages
	while (!used.empty() && used.front()->t < gvt)
	{
		used.pop_front(&free_msg_list);
	}
	// Delete old output
	Bag<X>* garbage = io_pool.make_obj();
	typename adevs::ulist<Message>::iterator discard_iter = discard.begin();
	while (discard_iter != discard.end())
	{
		if ((*discard_iter)->t < gvt)
		{
			garbage->insert((*discard_iter)->value);
			(*discard_iter)->src->gc_output(*garbage);
			garbage->clear();
			discard_iter = discard.erase(discard_iter,&free_msg_list);
		}
		else discard_iter++;
	}
	while (!intra_lp_output.empty() && intra_lp_output.front()->t < gvt) 
	{
		Message* msg = intra_lp_output.front();
		if (sim != NULL)
			sim->notify_output_listeners(msg->src,msg->value,msg->t.t);
		garbage->insert(msg->value);
		msg->src->gc_output(*garbage);
		garbage->clear();
		intra_lp_output.pop_front(&free_msg_list);
	}
	io_pool.destroy_obj(garbage);
	while (!inter_lp_output.empty() && inter_lp_output.front()->t < gvt) 
	{
		inter_lp_output.pop_front(&free_msg_list);
	}
}

template <class X>
bool LogicalProcess<X>::anti_message(adevs::ulist<Message>& l, Message* msg)
{
//	TAU_PROFILE("anti_message","void(list<Message<X> >&,Message<X>&)",TAU_DEFAULT);
	typename adevs::ulist<Message>::iterator msg_iter = l.begin();
	while (msg_iter != l.end())
	{
		if (msg->ID == -(*msg_iter)->ID && msg->src->lp == (*msg_iter)->src->lp)
		{
			l.erase(msg_iter,&free_msg_list);
			return true;
		}
		else msg_iter++;
	}
	return false;
}

template <class X>
void LogicalProcess<X>::processInput()
{
//	TAU_PROFILE("processInput","void (void)",TAU_DEFAULT);
	// Process all of the input messages. This method
	// performs rollbacks and message cancellations as
	// required.
	for (;;) // While there are input in the list
	{
		// Get the message at the front of the list or exit if no message is available
		bool patched = false;
		Message* msg;
		omp_set_lock(&lock);
		if ((msg = input.front()) != NULL)
		{
			input.pop_front();
		}
		omp_unset_lock(&lock);
		if (msg == NULL) break;
		// If this is a message in the past
		if (!chk_pt.empty() && msg->t <= chk_pt.back()->tf)
		{
			assert(avail.empty() || chk_pt.back()->tf <= avail.front()->t);
			perf_data.stragglers++;
			// If this is an input message then we can try to patch things up
			// quickly
			if (!(patched = patch(msg)))
			{
				// Cancel inter-LP output that occurred after the message time
				while (!inter_lp_output.empty() && msg->t < inter_lp_output.back()->t)
				{
					Message* antimsg = inter_lp_output.back();
					inter_lp_output.pop_back();
					// Send an anti-message to cancel inter-LP communications
					antimsg->ID *= -1;
					if (antimsg->t.t < send_min) send_min = antimsg->t.t;
					if (!antimsg->dst->lp->sendMessage(antimsg))
						free_msg_list.push_back(antimsg);
					perf_data.canceled_output++;
				}
				// Cancel intra-LP output that occurred at or after the message time
				while (!intra_lp_output.empty() && msg->t <= intra_lp_output.back()->t)
				{
					// Discard an intra-LP communication
					intra_lp_output.pop_back(&discard);
					perf_data.canceled_intra_lp_output++;
				}
				// If it is a rollback message, then remove it from the used list
				if (msg->ID < 0) anti_message(used,msg);
				// Move used messages at and after the event back to the available list
				while (!used.empty() && msg->t <= used.back()->t)
				{
					Message* msg_to_move = used.back();
					used.pop_back();
					avail.push_front(msg_to_move);
				}
				// Restore to a checkpoint that is in the past
				while (!chk_pt.empty() && msg->t <= chk_pt.back()->tf)
				{	
					Atomic<X>* model = chk_pt.back()->model;
					// Restore the state
					void* data = chk_pt.back()->data;
					model->tL = chk_pt.back()->ti;
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
					chk_pt.pop_back(&free_chk_pt_list);
					garbage_count--;
					perf_data.rollbacks++;
				}
			}
		}
		// Otherwise if it is a rollback message in the future
		// so just remove it from the available list 
		else if (msg->ID < 0)
		{
			anti_message(avail,msg);
		}
		// If it is not a rollback message and it is not in the used list
		if (msg->ID > 0 && !patched)
		{
			insert_message(avail,msg);
		}
		// Otherwise if it is an antimessage then save it for later use
		else if (msg->ID < 0) free_msg_list.push_back(msg);
	}
	assert(avail.empty() || used.empty() || used.back()->t < avail.front()->t);
	assert(!chk_pt.empty() || used.empty());
}

template <class X>
void LogicalProcess<X>::execEvents()
{
//	TAU_PROFILE("execEvents","void (void)",TAU_DEFAULT);
	Time tN = getNextEventTime();
	// If the next event is at infinity, then there is nothing to do
	if (tN.t == DBL_MAX) return;
	// Get the imminent models and route their output
	if (sched.minPriority() <= tN)
	{
		// Get the imminent messages
		sched.getImminent(imm);
		// Is this message a replay of an earlier message?
		inter_LP_ok = (inter_lp_output.empty() || inter_lp_output.back()->t < tN);
		// Compute and send the output values
		for (typename Bag<Atomic<X>*>::iterator imm_iter = imm.begin(); 
			imm_iter != imm.end(); imm_iter++)
		{
			Atomic<X>* model = *imm_iter;
			model->y = io_pool.make_obj();
			model->output_func(*(model->y));
			// Route each event in y
			for (typename Bag<X>::iterator y_iter = model->y->begin(); 
				y_iter != model->y->end(); y_iter++)
			{
				Message* msg = alloc_msg();
				// Local copy of the output
				msg->src = model;
				msg->t = tN;
				msg->value = *y_iter;
				intra_lp_output.push_back(msg);
				// Send messages to local models and other LPs
				route(model->getParent(),model,*y_iter);
			}
			// Clean up
			model->y->clear();
			io_pool.destroy_obj(model->y);
		}
	}
	// Route the external input that is in the avail list
	while (!avail.empty() && avail.front()->t <= tN)
	{
		Message* msg = avail.front();
		avail.pop_front();
		used.push_back(msg);
		inject_event(msg->dst->typeIsAtomic(),msg->value);
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
	CheckPoint* c = alloc_chk_pt();
	c->ti = model->tL;
	c->tf = t;
	c->model = model;
	c->data = model->save_state();
	chk_pt.push_back(c); 
	garbage_count++;
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
void LogicalProcess<X>::route(Network<X>* parent, Devs<X>* src, X& x, bool* inter_lp_msg)
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
				if (inter_lp_msg != NULL) *inter_lp_msg = true;
				Message* to_copy = intra_lp_output.back();
				Message* m_to_send = alloc_msg();
				Message* m_to_keep = alloc_msg();
				m_to_keep->t = m_to_send->t = to_copy->t;
				m_to_keep->src = m_to_send->src = to_copy->src;
				m_to_keep->ID = m_to_send->ID = ID++;
				assert(ID > 0);
				m_to_keep->dst = m_to_send->dst = amodel;
				m_to_keep->value = m_to_send->value = (*recv_iter).value;
				if (m_to_send->t.t < send_min) send_min = m_to_send->t.t;
				amodel->lp->sendMessage(m_to_send);
				inter_lp_output.push_back(m_to_keep);
			}
		}
		// if this is an external output from the parent model
		else if ((*recv_iter).model == parent)
		{
			route(parent->getParent(),parent,(*recv_iter).value,inter_lp_msg);
		}
		// otherwise it is an input to a coupled model
		else
		{
			route((*recv_iter).model->typeIsNetwork(),
				(*recv_iter).model,(*recv_iter).value,inter_lp_msg);
		}
	}
	// Free the bag of receivers
	recvs->clear();
	recv_pool.destroy_obj(recvs);
}

template <class X>
bool LogicalProcess<X>::sendMessage(Message* msg)
{
	bool insert_it = (msg->ID > 0);
	// Insert the message
	omp_set_lock(&lock);
	if (!insert_it) insert_it = !anti_message(input,msg);
	if (insert_it) input.push_back(msg);
	omp_unset_lock(&lock);
	if (!insert_it) perf_data.destroyed_in_input++;
	return insert_it;
}

template <class X>
void LogicalProcess<X>::insert_message(adevs::ulist<Message>& l, Message* msg)
{
//	TAU_PROFILE("insert_message","void(list<Message<X> >&,Message<X>&)",TAU_DEFAULT);
	typename adevs::ulist<Message>::iterator msg_iter;
	for (msg_iter = l.begin(); msg_iter != l.end(); msg_iter++)
	{
		if (msg->t <= (*msg_iter)->t) break;
	}
	l.insert(msg_iter,msg);
}

template <class X>
bool LogicalProcess<X>::patch(Message* msg)
{
	// If this is an anti-message we can't patch it
	if (msg->ID < 0) return false;
	// Get the target model
	Atomic<X>* model = msg->dst;
	// Is the message in the future of its target?
	if (model->tL < msg->t)
	{
		// If so, apply it tentatively
		void* data = model->save_state();
		model->x = io_pool.make_obj();
		model->x->insert(msg->value);
		model->delta_ext(msg->t.t-model->tL.t,*(model->x));
		model->x->clear();
		io_pool.destroy_obj(model->x);
		model->x = NULL;
		double h = model->ta();
		Time tN(Time::Inf());
		Time tL = msg->t + Time(0.0,1);
		if (h < DBL_MAX)
		{
			tN = tL + Time(h,0); 
			if (tN < tL) tN = tL;
		}
		// If the next event time is not in the past
		// then we are ok
		if (chk_pt.empty() || chk_pt.back()->tf < tN)
		{
			perf_data.patches++;
			sched.schedule(model,tN);
			CheckPoint* c = alloc_chk_pt();
			c->ti = model->tL;
			c->tf = msg->t;
			model->tL = tL;
			c->model = model;
			c->data = data;
			typename adevs::ulist<CheckPoint>::iterator iter;
			for (iter = chk_pt.begin(); iter != chk_pt.end(); iter++)
			{
				if (c->tf <= (*iter)->tf) break;
			}
			chk_pt.insert(iter,c);
			garbage_count++;
			insert_message(used,msg);
			return true;
		}
		// Otherwise our attempt at a patch failed
		else
		{
			model->restore_state(data);
			model->gc_state(data);
			return false;
		}
	}
	return false;
}

template <class X>
LogicalProcess<X>::~LogicalProcess()
{
	fossilCollect(Time::Inf(),NULL);
	// destroy lock
	omp_destroy_lock(&lock);
}

} // end of namespace 

#endif
