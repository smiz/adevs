/***************
Copyright (C) 2000-2010 by James Nutaro

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
#ifndef __adevs_event_listener_h_
#define __adevs_event_listener_h_
#include "adevs_models.h"
#include "adevs_bag.h"

namespace adevs
{

/**
 * The EventListener interface is used to receive output events produced
 * by model and to be notified of state changes at Atomic models.
 */
template <class X> class EventListener
{
	public:
		/**
		 * This callback is invoked when a model, network or atomic,
		 * produces an output. The default implementation is empty.
		 * @param x The model that produced the output and the output's value
		 * @param t The absolute time at which the output occurred
		 */
		virtual void outputEvent(Event<X> x, double t){}
		/**
		 * This callback is invoked by the simulator after an Atomic
		 * model changes its state. This method has an empty default
		 * implementation.
		 * @param model The model that changed state
		 * @param t The absolute time at which the state change occurred
		 */
		virtual void stateChange(Atomic<X>* model, double t){}
		/// Destructor
		virtual ~EventListener(){}
};

} // end of namespace

#endif
