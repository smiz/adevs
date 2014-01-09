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
#ifndef __adevs_models_h_
#define __adevs_models_h_
#include "adevs_time.h"
#include "adevs_bag.h"
#include "adevs_set.h"
#include "adevs_exception.h"
#include <cstdlib>

namespace adevs
{

/*
 * Declare network and atomic model so types can be used as the type of
 * parent in the basic Devs model and for type ID functions.  
 */
template <class X, class T> class Network;
template <class X, class T> class Atomic;
template <class X, class T> class Schedule;
template <class X, class T> class Simulator;

/*
 * Constant indicating no processor assignment for the model. This is used by the
 * parallel simulator
 */
#define ADEVS_NOT_ASSIGNED_TO_PROCESSOR -1

/**
 * The Devs class provides basic operations for all devs models.
 * The model I/O type is set by the template argument X. The 
 * type to be used for time is set with the template argument
 * T. The default type for time is double.
 */
template <class X, class T = double> class Devs
{
	public:
		/// Default constructor.
		Devs():
		parent(NULL),
		proc(ADEVS_NOT_ASSIGNED_TO_PROCESSOR)
		{
		}
		/// Destructor.
		virtual ~Devs()
		{
		}
		/**
		 * Returns NULL if this is not a network model; returns a pointer to
		 * itself otherwise. This method is used to avoid a relatively expensive
		 * dynamic cast.
		 */
		virtual Network<X,T>* typeIsNetwork() { return NULL; }
		/// Returns NULL if this is not an atomic model; returns itself otherwise.
		virtual Atomic<X,T>* typeIsAtomic() { return NULL; }
		/**
		 * Get the model that contains this model as a component.  Returns
		 * NULL if this model is at the top of the hierarchy.
		 */
		const Network<X,T>* getParent() const { return parent; }
		Network<X,T>* getParent() { return parent; }
		/**
		 * Assign a new parent to this model. Network model's should always
		 * call this method to make themselves the parent of their components.
		 * If the parent is not set correctly, then the event routing algorithm
		 * in the simulator will fail.
		 */
		void setParent(Network<X,T>* parent) { this->parent = parent; }
		/**
		 * This is the structure transition function, which is evaluated following
		 * every change of the model's state. It should return true
		 * if a structure change is to occur, and false otherwise. False is the
		 * default return value.
		 * This method is used by the simulator to limit the execution
		 * of potentially expensive structure changes. 
		 * If the return value is true, then the parent's model_transition()
		 * will also be evaluated. For network models, the model_transition() function is
		 * preceded and anteceded by a call to getComponents(). The difference
		 * of these two sets is used to determine if any models were added or removed
		 * as part of the model transition.
		 */
		virtual bool model_transition() { return false; }
		/**
		 * This method should return the model's lookahead, which is used by the parallel
		 * simulator to detect opportunities for parallel execution. The lookahead of a
		 * model (network or atomic) is the time into the future for which its output
		 * can be predicted without knowledge of the input to that that. This method
		 * returns zero by default.
		 */
		virtual T lookahead() { return adevs_zero<T>(); }
		/**
		 * This assigns the model to a processor on the parallel computer. If this is
		 * a network model, then its assignment will override the assignment of its
		 * components. If no assignment is made, then the atomic leaves of the model
		 * (or the model itself if it is already atomic) are assigned at random.
		 */
		void setProc(int proc) { this->proc = proc; }
		/**
		 * Get the processor assignment for this model. A negative number is returned
		 * if no assignment was made.
		 */
		int getProc() { return proc; }

	private:
		Network<X,T>* parent;
		int proc;
};

/**
 * Event objects are used for routing within a network model,
 * for notifying event listeners of output events, and for injecting
 * input into a running simulation.
 */
template <class X, class T = double> class Event
{
	public:
		/// Constructor.  Sets the model to NULL.
		Event():
		model(NULL),
		value()
		{
		}
		/**
		 * Constructor sets the model and value. The input into a
		 * Simulator and in a network's routing method,
		 * the model is the target of the input value.
		 * In a callback to an event listener, the model is the
		 * source of the output value. 
		 */
		Event(Devs<X,T>* model, const X& value):
		model(model),
		value(value)
		{
		}
		/// Copy constructor.
		Event(const Event<X,T>& src):
		model(src.model),
		value(src.value)
		{
		}
		/// Assignment operator.
		const Event<X,T>& operator=(const Event<X,T>& src)
		{
			model = src.model;
			value = src.value;
			return *this;
		}
		/// The model associated with the event.
		Devs<X,T>* model;
		/// The value associated with the event.
		X value;
		/// Destructor
		~Event()
		{
		}
};

/**
 * Base type for all atomic DEVS models.
 */
template <class X, class T = double> class Atomic: public Devs<X,T>
{
	public:
		/// The constructor should place the model into its initial state.
		Atomic():
		Devs<X,T>()
		{
			tL = adevs_zero<T>();
			tL_cp = adevs_sentinel<T>();
			x = y = NULL;
			q_index = 0; // The Schedule requires this to be zero
		}
		/// Internal transition function.
		virtual void delta_int() = 0;
		/**
		 * External transition function.
		 * @param e Time elapsed since the last change of state
		 * @param xb Input for the model.
		 */  
		virtual void delta_ext(T e, const Bag<X>& xb) = 0;
		/**
		 * Confluent transition function.
		 * @param xb Input for the model.
		 */
		virtual void delta_conf(const Bag<X>& xb) = 0;
		/**
		 * Output function.  Output values should be added to the bag yb.
		 * @param yb Empty bag to be filled with the model's output
		 */
		virtual void output_func(Bag<X>& yb) = 0;
		/**
		 * Time advance function. adevs_inf<T>() is used for infinity.
		 * @return The time to the next internal event
		 */
		virtual T ta() = 0;
		/**
		 * Garbage collection function.  The objects in g are
		 * no longer in use by the simulation engine and should be disposed of. 
`		 * Note that the elements in g are only those objects produced as
		 * output by this model.
		 */
		virtual void gc_output(Bag<X>& g) = 0;
		/**
		 * This method is called by the simulator just before the model
		 * is used in a lookahead calculation. When this method is called
		 * the model must perform in such a way as to be able to restore
		 * itself to its current state when the restore() method is called
		 * at the end of the lookahead calculation. If this method is not
		 * supported then it must throw a method_not_supported_exception,
		 * which is the default.
		 */
		virtual void beginLookahead()
		{
			method_not_supported_exception ns("beginLookahead",this);
			throw ns;
		}
		/**
		 * This method is called when a lookahead calculation is finished.
		 * The model must restore its state to that which it was in when
		 * beginLookahead was called. The default implementation is to
		 * do nothing.
		 */
		virtual void endLookahead(){}
		/// Destructor.
		virtual ~Atomic(){}
		/// Returns a pointer to this model.
		Atomic<X,T>* typeIsAtomic() { return this; }
	protected:
		/**
		 * Get the last event time for this model. This is 
		 * provided primarily for use with the backwards compatibility
		 * module and should not be relied on. It is likely to be
		 * removed in later versions of the code.
		 */
		T getLastEventTime() const { return tL; }

	private:

		friend class Simulator<X,T>;
		friend class Schedule<X,T>;

		// Time of last event
		T tL;
		// Index in the priority queue
		unsigned int q_index;
		// Input and output event bags
		Bag<X> *x, *y;
		// When did the model start checkpointing?
		T tL_cp;
};

/**
 * Base class for DEVS network models.
 */
template <class X, class T = double> class Network: public Devs<X,T>
{
	public:
		/// Constructor.
		Network():
		Devs<X,T>()
		{
		}
		/**
		 * This method should fill the
		 * set c with all the Network's components, excluding the 
		 * Network model itself.
		 * @param c An empty set to the filled with the Network's components.
		 */
		virtual void getComponents(Set<Devs<X,T>*>& c) = 0;
		/**
		 * This method is called by the Simulator to route an output value
		 * produced by a model. This method should fill the bag r
		 * with Events that point to the target model and carry the value
		 * to be delivered to the target. The target may be a component 
		 * of the Network or the Network itself, the latter causing the
		 * Network to produce an output.
		 * @param model The model that produced the output value
		 * @param value The output value produced by the model
		 * @param r A bag to be filled with (target,value) pairs
		 */
		virtual void route(const X& value, Devs<X,T>* model, Bag<Event<X,T> >& r) = 0;
		/**
		 * Destructor.  This destructor does not delete any component models.
		 * Any necessary cleanup should be done by the derived class.
		 */
		virtual ~Network()
		{
		}
		/// Returns a pointer to this model.
		Network<X,T>* typeIsNetwork() { return this; }
};

} // end of namespace

#endif
