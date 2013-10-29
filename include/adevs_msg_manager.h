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
#ifndef __adevs_msg_manager_h_
#define __adevs_msg_manager_h_

namespace adevs
{

/**
 * This class is for managing memory used by objects that are
 * passed between processors in a parallel simulation. 
 */
template <typename X> class MessageManager
{
	public:
		/**
		 * Create a copy of the value. This is called by the simulator
		 * when the value object is about to leave a thread boundary.
		 */
		virtual X clone(X& value) = 0;
		/**
		 * Free the value. This is called when the thread that received
		 * the object is done with it. Note that the original object
		 * may or may not have been deleted when this method is finally
		 * called for the copy.
		 */
		virtual void destroy(X& value) = 0;
		virtual ~MessageManager(){}
};

/**
 * This is the default MessageManager that is used by the 
 * parallel simulator if an alternative is not provided.
 */
template <typename X> class NullMessageManager:
	public MessageManager<X>
{
	public:
		/// Uses the objects default copy constructor
		X clone(X& value) { return value; }
		/// Takes no action on the value
		void destroy(X& value){}
};

}

#endif
