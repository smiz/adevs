/***************
Copyright (C) 2000-2006 by James Nutaro

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

#ifndef __adevs_object_pool_h_
#define __adevs_object_pool_h_
#include "adevs_bag.h"

namespace adevs
{

/**
A utility class for managing pools of objects that are stored on the heap.
Uses the new and delete operators to create and destroy objects.
*/
template <class T> class object_pool
{
	public:
		// Construct a pool with a specific initial population
		object_pool(unsigned long int pop = 0):
		pool()
		{
			for (unsigned long int i = 0; i < pop; i++)
			{
				pool.insert(new T());
			}
		}
		// Create an object 
		T* make_obj()
		{
			T* obj;
			if (pool.empty())
			{
				obj = new T;
			}
			else
			{
				obj = *((pool.end())--);
				pool.erase(pool.end()--);
			}
			return obj;
		}
		// Return an object to the pool
		void destroy_obj(T* obj)
		{
			pool.insert(obj);
		}
		// Delete all objects in the pool
		~object_pool()
		{
			typename Bag<T*>::iterator iter = pool.begin();
			for (; iter != pool.end(); iter++)
			{
				delete *iter;
			}
		}
	private:
		Bag<T*> pool;
};

} // end of namespace

#endif
