/***************
Copyright (C) 2007 by James Nutaro

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
	 * as output from teh ModelWrapper. If the wrapped model is a Network, the input
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
	template <typename ExternalType, typename InternalType> class ModelWrapper:
		public Atomic<ExternalType>,
		public EventListener<InternalType>
	{
		public:
			/**
			 * Create a wrapper for the specified model. The ModelWrapper takes
			 * ownership of the supplied model and will delete it when the
			 * ModelWrapper is deleted.
			 */
			ModelWrapper(Devs<InternalType>* model);
			/**
			 * This method is used to translate incoming input objects into
			 * input objects that the wrapped model can process. The supplied
			 * internal_input bag should be filled with Events that contain the targeted
			 * internal models and the values to supply to them. The external_input
			 * bag contains the input values supplied to the wrapper's external or
			 * confluent transition function.
			 */
			virtual void translateInput(const Bag<ExternalType>& external_input,
					Bag<Event<InternalType> >& internal_input) = 0;
			/**
			 * This method is used to translate outgoing output objects
			 * into objects that the ModelWrapper can produce. The 
			 * internal_output bag contains all of the output events that the
			 * were produced by the wrapped model. The external_output bag
			 * should be filled with objects of type ExternalType that
			 * will be produced as output by the ModelWrapper.
			 */
			virtual void translateOutput(const Bag<Event<InternalType> >& internal_output,
					Bag<ExternalType>& external_output) = 0;
			/**
			 * This is the garbage collection method for internal input events.
			 * It will be called when the wrapper is done with a set of events
			 * that you created with the translateInput method. The supplied bag
			 * is the same one that you filled out in the translateInput method.
			 */
			virtual void gc_input(Bag<Event<InternalType> >& g) = 0;
			/// Get the model that is wrapped by this object
			Devs<InternalType>* getWrappedModel() { return model; }
			/// Atomic internal transition function
			void delta_int();
			/// Atomic external transition function
			void delta_ext(double e, const Bag<ExternalType>& xb);
			/// Atomic confluent transition function
			void delta_conf(const Bag<ExternalType>& xb);
			/// Atomic output function
			void output_func(Bag<ExternalType>& yb);
			/// Atomic time advance function
			double ta();
			/// EventListener outputEvent method
			void outputEvent(Event<InternalType> y, double t);
			/// Destructor. This destroys the wrapped model too.
			~ModelWrapper();
		private:
			ModelWrapper(){}
			ModelWrapper(const ModelWrapper&){}
			void operator=(const ModelWrapper&){}
			// Bag of events created by the input translation method 
			Bag<Event<InternalType> > input;
			// Output from the wrapped model
			Bag<Event<InternalType> > output;
			// The wrapped model
			Devs<InternalType>* model;
			// Simulator for driving the wrapped model
			Simulator<InternalType>* sim;
			// Last event time
			double tL;
	};

template <typename ExternalType, typename InternalType> 
ModelWrapper<ExternalType,InternalType>::ModelWrapper(Devs<InternalType>* model):
	Atomic<ExternalType>(),
	EventListener<InternalType>(),
	model(model),
	tL(0.0)
{
	sim = new Simulator<InternalType>(model);
	sim->addEventListener(this);
}

template <typename ExternalType, typename InternalType> 
void ModelWrapper<ExternalType,InternalType>::delta_int()
{
	// Update the internal clock
	tL = sim->nextEventTime();
	// Execute the next autonomous event for the wrapped model
	sim->execNextEvent();
}

template <typename ExternalType, typename InternalType> 
void ModelWrapper<ExternalType,InternalType>::delta_ext(double e, const Bag<ExternalType>& xb)
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

template <typename ExternalType, typename InternalType> 
void ModelWrapper<ExternalType,InternalType>::delta_conf(const Bag<ExternalType>& xb)
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

template <typename ExternalType, typename InternalType> 
double ModelWrapper<ExternalType,InternalType>::ta()
{
	if (sim->nextEventTime() < DBL_MAX) return sim->nextEventTime()-tL;
	else return DBL_MAX;
}

template <typename ExternalType, typename InternalType> 
void ModelWrapper<ExternalType,InternalType>::output_func(Bag<ExternalType>& yb)
{
	// Compute the model's output events; this causes the outputEvent method to be called
	sim->computeNextOutput();
	// Translate the output events to external output events
	translateOutput(output,yb);
	// Clean up; the contents of the output bag are deleted by the wrapped model's 
	// gc_output method
	output.clear();
}

template <typename ExternalType, typename InternalType> 
void ModelWrapper<ExternalType,InternalType>::outputEvent(Event<InternalType> y, double t)
{
	// Just save the events for processing by the output_func
	output.insert(y);
}

template <typename ExternalType, typename InternalType> 
ModelWrapper<ExternalType,InternalType>::~ModelWrapper()
{
	delete sim;
	delete model;
}

} // end of namespace

#endif
