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
template <typename X, typename T> class Network;
template <typename X, typename T> class Atomic;
template <typename X, typename T> class MealyAtomic;
template <typename X, typename T> class Schedule;
template <typename X, typename T> class Simulator;

/**
 * The Devs class provides basic operations for all devs models.
 * The model I/O type is set by the template argument X. The 
 * type to be used for time is set with the template argument
 * T. The default type for time is double.
 */
template <typename X, typename T = double> class Devs
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
		 * Returns NULL if this is not a network model; returns a pointer to
		 * itself otherwise. This method is used to avoid a relatively expensive
		 * dynamic cast.
		 */
		virtual Network<X,T>* typeIsNetwork() { return NULL; }
		/// Returns NULL if this is not an atomic model; returns itself otherwise.
		virtual Atomic<X,T>* typeIsAtomic() { return NULL; }
		/// Returns NULL if this is not a mealy atomic model; returns itself otherwise.
		virtual MealyAtomic<X,T>* typeIsMealyAtomic() { return NULL; }
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
	private:
		Network<X,T>* parent;
};

/**
 * Event objects are used for routing within a network model,
 * for notifying event listeners of output events, and for injecting
 * input into a running simulation.
 */
template <typename X, typename T = double> class Event
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
template <typename X, typename T = double> class Atomic: public Devs<X,T>
{
	public:
		/// The constructor should place the model into its initial state.
		Atomic():
			Devs<X,T>(),
			tL(adevs_zero<T>()),
			q_index(0), // The Schedule requires this to be zero
			proc(-1),
			x(NULL),
			y(NULL)
		{
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
		// Thread assigned to this model
		int proc;
		// Input and output event bags
		Bag<X> *x, *y;
};

/**
 * This is a Mealy type atomic model where its output
 * may depend on its input. Mealy machines cannot be
 * connected to other Mealy machines. An exception
 * will be generated by the Simulator if you attempt
 * to do so.
 */

template <typename X, typename T = double> class MealyAtomic:
	public Atomic<X,T>
{
	public:
		MealyAtomic<X,T>():Atomic<X,T>(),imm(false){}
		MealyAtomic<X,T>* typeIsMealyAtomic() { return this; }
		/**
		 * Produce output at e < ta(q) in response to xb.
		 * This is output preceding an external event.
		 */
		virtual void output_func(T e, const Bag<X>& xb, Bag<X>& yb) = 0;
		/**
		 * Produce output at e = ta(q) in response to xb.
		 * This is output preceding a confluent event.
		 */
		virtual void output_func(const Bag<X>& xb, Bag<X>& yb) = 0;
		virtual ~MealyAtomic(){}

	private:

		friend class Simulator<X,T>;

		bool imm;
};

/**
 * Base class for DEVS network models.
 */
template <typename X, typename T = double> class Network: public Devs<X,T>
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
