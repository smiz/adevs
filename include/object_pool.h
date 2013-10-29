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
#ifndef __adevs_object_pool_h_
#define __adevs_object_pool_h_
#include "adevs_bag.h"

namespace adevs
{

/**
 * A utility class for managing pools of objects that are stored on the heap.
 * Uses the new and delete operators to create and destroy objects.
 */
template <class T> class object_pool
{
	public:
		/// Construct a pool with a specific initial population
		object_pool(unsigned int pop = 0):
		pool()
		{
			for (unsigned int i = 0; i < pop; i++)
				pool.insert(new T());
		}
		/// Create an object 
		T* make_obj()
		{
			T* obj;
			if (pool.empty()) obj = new T;
			else
			{
				obj = *((pool.end())--);
				pool.erase(pool.end()--);
			}
			return obj;
		}
		/// Return an object to the pool
		void destroy_obj(T* obj) { pool.insert(obj); }
		// Delete all objects in the pool
		~object_pool()
		{
			typename Bag<T*>::iterator iter = pool.begin();
			for (; iter != pool.end(); iter++) delete *iter;
		}
	private:
		Bag<T*> pool;
};

} // end of namespace

#endif
