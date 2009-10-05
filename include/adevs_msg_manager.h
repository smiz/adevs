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
