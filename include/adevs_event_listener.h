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
template <class X, class T = double> class EventListener
{
	public:
		/**
		 * This callback is invoked when a model, network or atomic,
		 * produces an output. The default implementation is empty.
		 * @param x The model that produced the output and the output's value
		 * @param t The absolute time at which the output occurred
		 */
		virtual void outputEvent(Event<X,T> x, T t){}
		/**
		 * This callback is invoked by the simulator after an Atomic
		 * model changes its state. This method has an empty default
		 * implementation.
		 * @param model The model that changed state
		 * @param t The absolute time at which the state change occurred
		 */
		virtual void stateChange(Atomic<X,T>* model, T t){}
		/// Destructor
		virtual ~EventListener(){}
};

} // end of namespace

#endif
