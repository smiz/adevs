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
template <class X, class T = double> class AbstractSimulator 
{
	public:
		AbstractSimulator(){}
		/**
		 * Add an event listener that will be notified of output events 
		 * produced by the model.
		 */
		void addEventListener(EventListener<X,T>* l)
		{
			listeners.insert(l);
		}
		/// Remove an event listener
		void removeEventListener(EventListener<X,T>* l)
		{
			listeners.erase(l);
		}
		/// Get the model's next event time
		virtual T nextEventTime() = 0;
		/// Execute the simulator until the next event time is greater than tend
		virtual void execUntil(T tend) = 0;
		/// Destructor leaves the model intact.
		virtual ~AbstractSimulator(){}
		/// Notify listeners of an output event.
		void notify_output_listeners(Devs<X,T>* model, const X& value, T t);
		/// Notify listeners of a state change.
		void notify_state_listeners(Atomic<X,T>* model, T t);
	private:
		/// Eternal event listeners
		Bag<EventListener<X,T>*> listeners;

};

template <class X, class T>
void AbstractSimulator<X,T>::notify_output_listeners(Devs<X,T>* model, const X& value, T t)
{
	Event<X,T> event(model,value);
	typename Bag<EventListener<X,T>*>::iterator iter;
	for (iter = listeners.begin(); iter != listeners.end(); iter++)
	{
		(*iter)->outputEvent(event,t);
	}
}

template <class X, class T>
void AbstractSimulator<X,T>::notify_state_listeners(Atomic<X,T>* model, T t)
{
	typename Bag<EventListener<X,T>*>::iterator iter;
	for (iter = listeners.begin(); iter != listeners.end(); iter++)
	{
		(*iter)->stateChange(model,t);
	}
}

} // end of namespace

#endif
