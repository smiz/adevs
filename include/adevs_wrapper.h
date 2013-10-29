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
#ifndef __adevs_wrapper_h_
#define __adevs_wrapper_h_
#include "adevs_models.h"

namespace adevs
{
	/**
	 * <p>This class wraps a Network or Atomic model with interface type InternalType in an
	 * Atomic model with interface type ExternalType. Input to the ModelWrapper is passed
	 * through a user provided input translation method before being handed off
	 * to the wrapped model for processing. Output from the wrapped model is
	 * passed through a user provided output translation method before emerging
	 * as output from the ModelWrapper. If the wrapped model is a Network, the input
	 * translation method can create inputs for any of its components. Similarly
	 * the output translation method is provided with every output produced by
	 * every component in the Network. If the wrapped model is Atomic then there
	 * is, of course, only one possible destination for incoming events and only
	 * one source of outgoing events.
	 * <p>You will need to implement the usual gc_output event for outputs
	 * produced by the ModelWrapper. You will also need to implement
	 * gcInputEvents method to clean up events that are created during
	 * the input translation process.
	 */
	template <typename ExternalType, typename InternalType, class T = double> class ModelWrapper:
		public Atomic<ExternalType,T>,
		public EventListener<InternalType,T>
	{
		public:
			/**
			 * Create a wrapper for the specified model. The ModelWrapper takes
			 * ownership of the supplied model and will delete it when the
			 * ModelWrapper is deleted.
			 */
			ModelWrapper(Devs<InternalType,T>* model);
			/**
			 * This method is used to translate incoming input objects into
			 * input objects that the wrapped model can process. The supplied
			 * internal_input bag should be filled with Events that contain the targeted
			 * internal models and the values to supply to them. The external_input
			 * bag contains the input values supplied to the wrapper's external or
			 * confluent transition function.
			 */
			virtual void translateInput(const Bag<ExternalType>& external_input,
					Bag<Event<InternalType,T> >& internal_input) = 0;
			/**
			 * This method is used to translate outgoing output objects
			 * into objects that the ModelWrapper can produce. The 
			 * internal_output bag contains all of the output events that the
			 * were produced by the wrapped model. The external_output bag
			 * should be filled with objects of type ExternalType that
			 * will be produced as output by the ModelWrapper.
			 */
			virtual void translateOutput(const Bag<Event<InternalType,T> >& internal_output,
					Bag<ExternalType>& external_output) = 0;
			/**
			 * This is the garbage collection method for internal input events.
			 * It will be called when the wrapper is done with a set of events
			 * that you created with the translateInput method. The supplied bag
			 * is the same one that you filled out in the translateInput method.
			 */
			virtual void gc_input(Bag<Event<InternalType,T> >& g) = 0;
			/// Get the model that is wrapped by this object
			Devs<InternalType,T>* getWrappedModel() { return model; }
			/// Atomic internal transition function
			void delta_int();
			/// Atomic external transition function
			void delta_ext(T e, const Bag<ExternalType>& xb);
			/// Atomic confluent transition function
			void delta_conf(const Bag<ExternalType>& xb);
			/// Atomic output function
			void output_func(Bag<ExternalType>& yb);
			/// Atomic time advance function
			T ta();
			/// EventListener outputEvent method
			void outputEvent(Event<InternalType,T> y, T t);
			/// Destructor. This destroys the wrapped model too.
			~ModelWrapper();
		private:
			ModelWrapper(){}
			ModelWrapper(const ModelWrapper&){}
			void operator=(const ModelWrapper&){}
			// Bag of events created by the input translation method 
			Bag<Event<InternalType,T> > input;
			// Output from the wrapped model
			Bag<Event<InternalType,T> > output;
			// The wrapped model
			Devs<InternalType,T>* model;
			// Simulator for driving the wrapped model
			Simulator<InternalType,T>* sim;
			// Last event time
			T tL;
	};

template <typename ExternalType, typename InternalType, class T> 
ModelWrapper<ExternalType,InternalType,T>::ModelWrapper(Devs<InternalType,T>* model):
	Atomic<ExternalType,T>(),
	EventListener<InternalType,T>(),
	model(model),
	tL(adevs_zero<T>())
{
	sim = new Simulator<InternalType,T>(model);
	sim->addEventListener(this);
}

template <typename ExternalType, typename InternalType, class T> 
void ModelWrapper<ExternalType,InternalType,T>::delta_int()
{
	// Update the internal clock
	tL = sim->nextEventTime();
	// Execute the next autonomous event for the wrapped model
	sim->execNextEvent();
}

template <typename ExternalType, typename InternalType, class T> 
void ModelWrapper<ExternalType,InternalType,T>::delta_ext(T e, const Bag<ExternalType>& xb)
{
	// Update the internal clock
	tL += e;
	// Convert the external inputs to internal inputs
	translateInput(xb,input);
	// Apply the input
	sim->computeNextState(input,tL);
	// Clean up 
	gc_input(input);
	input.clear();
}

template <typename ExternalType, typename InternalType, class T> 
void ModelWrapper<ExternalType,InternalType,T>::delta_conf(const Bag<ExternalType>& xb)
{
	// Update the internal clock
	tL = sim->nextEventTime();
	// Convert the external inputs to internal inputs
	translateInput(xb,input);
	// Apply the input
	sim->computeNextState(input,tL);
	// Clean up 
	gc_input(input);
	input.clear();
}

template <typename ExternalType, typename InternalType, class T> 
T ModelWrapper<ExternalType,InternalType,T>::ta()
{
	if (sim->nextEventTime() < adevs_inf<T>()) return sim->nextEventTime()-tL;
	else return adevs_inf<T>();
}

template <typename ExternalType, typename InternalType, class T> 
void ModelWrapper<ExternalType,InternalType,T>::output_func(Bag<ExternalType>& yb)
{
	// Compute the model's output events; this causes the outputEvent method to be called
	sim->computeNextOutput();
	// Translate the output events to external output events
	translateOutput(output,yb);
	// Clean up; the contents of the output bag are deleted by the wrapped model's 
	// gc_output method
	output.clear();
}

template <typename ExternalType, typename InternalType, class T> 
void ModelWrapper<ExternalType,InternalType,T>::outputEvent(Event<InternalType,T> y, T t)
{
	// Just save the events for processing by the output_func
	output.insert(y);
}

template <typename ExternalType, typename InternalType, class T> 
ModelWrapper<ExternalType,InternalType,T>::~ModelWrapper()
{
	delete sim;
	delete model;
}

} // end of namespace

#endif
