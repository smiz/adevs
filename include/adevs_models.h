/***************
Copyright (C) 2000-2006 by James Nutaro

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
#ifndef __adevs_models_h_
#define __adevs_models_h_
#include "adevs_bag.h"
#include "adevs_set.h"
#include "adevs_time.h"
#include <cstdlib>

namespace adevs
{

/*
Predeclare network and atomic model so types can be used as the type of
parent in the basic devs model and for type ID functions.  
*/
template <class X> class Network;
template <class X> class Atomic;
template <class X, class T> class Schedule;
template <class X> class Simulator;
// These predeclerations are needed to support the optimistic simulator
template <class X> class OptSimulator;
template <class X> class LogicalProcess;

/**
The Devs class provides basic operations for all devs models.
The model I/O type can be specialized with the the template argument.
*/
template <class X> class Devs
{
	public:
		/// Default constructor.
		Devs():
		parent(NULL)
		{
		}
		/// Destructor.
		virtual ~Devs()
		{
		}
		/**
		Returns NULL if this is not a network model; returns a pointer to
		itself otherwise.  This method is used to avoid a relatively expensive
		dynamic cast.
		*/
		virtual Network<X>* typeIsNetwork() { return NULL; }
		/// Returns NULL if this is not a atomic model; returns itself otherwise.
		virtual Atomic<X>* typeIsAtomic() { return NULL; }
		/**
		Get the model that contains this model as a component.  Returns
		NULL if this model is at the top of the hierarchy.
		*/
		const Network<X>* getParent() const { return parent; }
		Network<X>* getParent() { return parent; }
		/**
		Assign a new parent to this model.  This method is intended to
		support end user network models.
		*/
		void setParent(Network<X>* parent) { this->parent = parent; }
		/**
		This is the model transition function.  It should return true
		if a model transition occurs, and false otherwise. False is the
		default return value.	
		This method is used by the simulator to limit the execution
		of potentially expensive structure changes. 
		If the return value is true, then the parent's model_transition()
		will be evaluated. For Network models, the model_transition() function is
		preceded and anteceded by a call to getComponents(). The difference
		of these two sets is used to determine if any models were added or removed
		as part of the model transition.
		*/
		virtual bool model_transition() { return false; }
		
	private:
		Network<X>* parent;
};

/**
Event objects are used for routing and notification of external simulation
event listeners.
*/
template <class X> class Event
{
	public:
		/// Constructor.  Sets the model to NULL.
		Event():
		model(NULL),
		value()
		{
		}
		/// Constructor that sets the model and value.
		Event(Devs<X>* model, const X& value):
		model(model),
		value(value)
		{
		}
		/// Copy constructor.
		Event(const Event<X>& src):
		model(src.model),
		value(src.value)
		{
		}
		/// Assignment operator.
		const Event<X>& operator=(const Event<X>& src)
		{
			model = src.model;
			value = src.value;
			return *this;
		}
		/// The model associated with the event.
		Devs<X>* model;
		/// The value associated with the event.
		X value;
		/// Destructor
		~Event()
		{
		}
};

/**
Base type for all atomic DEVS models.
*/
template <class X> class Atomic: public Devs<X>
{
	public:
		/**
		The constructor should place the model into its initial state.
		*/
		Atomic():
		Devs<X>()
		{
			tL = 0.0;
			x = y = NULL;
			q_index = 0; // The Schedule requires this to be zero
			active = false;
			lp = NULL; // This is created by the OptSimulator if it is needed
		}
		/// Internal transition function.
		virtual void delta_int() = 0;
		/// External transition function.  
		virtual void delta_ext(double e, const Bag<X>& xb) = 0;
		/// Confluent transition function.
		virtual void delta_conf(const Bag<X>& xb) = 0;
		/// Output function.  Output values should be added to the bag y.
		virtual void output_func(Bag<X>& yb) = 0;
		/// Time advance function. DBL_MAX is used as infinity.
		virtual double ta() = 0;
		/**
		Garbage collection function.  The objects in g are
		no longer in use by the simulation engine and should be disposed of. 
`		Note that the elements in g are only those objects produced as
		output by this model.
		*/
		virtual void gc_output(Bag<X>& g) = 0;
		/**
		 * Make a copy of the model's current state and return it as a pointer to
		 * void. This is used by the optimistic simulator for making simulation
		 * checkpoints. The default implementation returns NULL.
		 */
		virtual void* save_state() { return NULL; }
		/**
		 * Restore a state that was previously obtained with a call to save_state().
		 * This is used by the optimistic simulator to restore a simulation checkpoint.
		 * The default implementation does nothing.
		 */
		virtual void restore_state(void* state){}
		/**
		 * Delete a checkpoint that was created previously by calling save_state().
		 * This is used by the optimistic simulator to do fossil collection. The
		 * default implementation does nothing.
		 */
		virtual void gc_state(void* old_state){}
		/// Destructor.
		virtual ~Atomic()
		{
		}
		/// Returns a pointer to this model.
		Atomic<X>* typeIsAtomic() { return this; }

	protected:
		/**
		Get the last event time for this model. This is 
		provided primarily for use with the backwards compatibility
		functions and should not be relied on. It is likely to be
		removed in later versions of the code.
		*/
		double getLastEventTime() const { return tL.t; }

	private:

		friend class Simulator<X>;
		friend class OptSimulator<X>;
		friend class Schedule<X,double>;
		friend class Schedule<X,Time>;

		// Time of last event
		Time tL;
		// Index in the priority queue
		unsigned int q_index;
		// Input and output event bags
		Bag<X> *x, *y;
		// The logical process that this model is assigned to
		LogicalProcess<X>* lp;
		// Has this model been actived?
		bool active;
};

/**
Base class for DEVS network models.
*/
template <class X> class Network: public Devs<X>
{
	public:
		/// Constructor.
		Network():
		Devs<X>()
		{
		}
		/**
		Implementations of this method should fill the
		set c with all components models, excluding the model
		Network model itself.
		*/
		virtual void getComponents(Set<Devs<X>*>& c) = 0;
		/**
		An implementation should fill the EventReceiver bag r
		with all Events that describe the target model and value
		to be delivered to the target. 
		*/
		virtual void route(const X& value, Devs<X>* model, Bag<Event<X> >& r) = 0;
		/**
		Destructor.  This destructor does not delete any component models.
		Any cleanup should be done by the derived class.
		*/
		virtual ~Network()
		{
		}
		/// Returns a pointer to this model.
		Network<X>* typeIsNetwork() { return this; }
};

} // end of namespace

#endif
