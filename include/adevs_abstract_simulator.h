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
#ifndef __adevs_abstract_simulator_h_
#define __adevs_abstract_simulator_h_
#include "adevs_models.h"
#include "adevs_event_listener.h"
#include "adevs_bag.h"

namespace adevs
{

/**
 * This is the base class for all simulators. It defines an interface that is
 * supported by all derived classes and provides some basic helper routines
 * for those derived classes.
 */
template <class X> class AbstractSimulator 
{
	public:
		/**
		Create a simulator for the provided model. The simulator
		constructor will fail and throw an adevs::exception if the
		time advance of any component atomic model is less than zero.
		*/
		AbstractSimulator()
		{
		}
		/**
		Add an event listener that will be notified of output events 
		produced by the model.
		*/
		void addEventListener(EventListener<X>* l)
		{
			listeners.insert(l);
		}
		/// Remove an event listener
		void removeEventListener(EventListener<X>* l)
		{
			listeners.erase(l);
		}
		/// Get the model's next event time
		virtual double nextEventTime() = 0;
		/// Execute the simulator until the next event is greater than tend
		virtual void execUntil(double tend) = 0;
		/// Destructor should leave the model intact.
		virtual ~AbstractSimulator(){}
		/// Notify listeners of an output event.
		void notify_output_listeners(Devs<X>* model, const X& value, double t);
		/// Notify listeners of a state change.
		void notify_state_listeners(Atomic<X>* model, double t, void* value = NULL);
	private:
		/// Eternal event listeners
		Bag<EventListener<X>*> listeners;

};

template <class X>
void AbstractSimulator<X>::notify_output_listeners(Devs<X>* model, const X& value, double t)
{
	Event<X> event(model,value);
	typename Bag<EventListener<X>*>::iterator iter;
	for (iter = listeners.begin(); iter != listeners.end(); iter++)
	{
		(*iter)->outputEvent(event,t);
	}
}

template <class X>
void AbstractSimulator<X>::notify_state_listeners(Atomic<X>* model, double t, void* value)
{
	typename Bag<EventListener<X>*>::iterator iter;
	for (iter = listeners.begin(); iter != listeners.end(); iter++)
	{
		(*iter)->stateChange(model,t,value);
	}
}

} // End of namespace

#endif
