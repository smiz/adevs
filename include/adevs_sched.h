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
#ifndef __adevs_schedule_h_
#define __adevs_schedule_h_
#include "adevs_models.h"
#include <cfloat>
#include <cstdlib>

namespace adevs
{

/**
A binary heap that is used for scheduling atomic models in the
simulation engine.  The atomic model q_index is used to store
the position of the model in the heap. A zero value indicate
that the model is not in the heap, and the initial value of any
model should be q_index = 0.
*/
template <class X, class T=double> class Schedule
{
	public:
		/**
		Creates a scheduler with the default or specified initial capacity.
		*/
		Schedule(unsigned long int capacity = 100):
		capacity(capacity),
		size(0)
		{
			heap = new heap_element[capacity];
		}
		/**
		Add a model to the schedule or change to position of a model already in
		the schedule. If the tN is DBL_MAX, then the item is removed
		(if it is already in the queue) or not inserted.  Because this method
		uses the model's q_index field, it is imperative that a model be 
		entered into one, and only one, schedule at any time.
		*/
		void schedule(Atomic<X>* model, T priority);
		/**
		Remove the model at the front of the queue.
		*/
		void removeMinimum();
		/**
		Get the model at the front of the queue.
		*/
		Atomic<X>* getMinimum() const
		{
			return heap[1].item;
		}
		/**
		Get the imminent models and set their active flags to true.
		*/
		void getImminent(Bag<Atomic<X>*>& imm) const
		{
			getImminent(imm,1);
		}
		/**
		Remove the imminent models from the queue.
		*/
		void removeImminent();
		/**
		Returns the smallest time of next event, or DBL_MAX if
		the queue is empty.
		*/
		T minPriority() const 
		{ 
			return heap[1].priority; 
		}
		/// Returns true if the queue is empty, and false otherwise.
		bool empty() const 
		{ 
			return size == 0; 
		}
		/**
		 * Get the model at position k in the heap. The value of k must be between
		 * 1 and size inclusive.
		 */
		Atomic<X>* get(unsigned long int k) const
		{
			return heap[k].item;
		}
		/**
		 * Get the number of elements in the heap.
		 */
		unsigned long int getSize() const
		{
			return size;
		}
		/// Destructor.
		~Schedule()
		{
			delete [] heap;
		}

	private:
		// Definition of an element in the heap.
		class heap_element 
		{
			public:
				Atomic<X>* item;
				T priority;
				heap_element():
				item(NULL),
				priority(DBL_MAX)
				{
				}
				heap_element(X* item, T priority):
				item(item),
				priority(priority)
				{
				}
				heap_element(const heap_element& src):
				item(src.item),
				priority(src.priority)
				{
				}
				const heap_element& operator=(const heap_element& src)
				{
					item = src.item;
					priority = src.priority;
					return *this;
				}
				~heap_element()
				{
				}
		};
		heap_element* heap;
		unsigned long int capacity;
		unsigned long int size;

		/// Double the schedule capacity
		void enlarge();
		/**
		Heap percolation functions.  Returns the empty slot that emerges for
		a new item with the provided priority.
		*/
		unsigned long int percolate_down(unsigned long int index, T priority);
		unsigned long int percolate_up(unsigned long int index, T priority);
		/// Construct the imminent set recursively
		void getImminent(Bag<Atomic<X>*>& imm, unsigned long int root) const;
};

template <class X,class T>
void Schedule<X,T>::schedule(Atomic<X>* model, T priority)
{
	/**
	Check for existance in the queue. 
	*/
	bool enqueued = false;
	if (model->q_index != 0)
	{
		enqueued = (heap[model->q_index].item == model);
	}
	/**
	If it is being added, then increment the size and check for
	capacity overflow.  Once this is done, actually add the model to the
	schedule.
	*/
	if (!enqueued && priority < DBL_MAX)
	{
		size++;
		if (size == capacity)
		{
			enlarge();
		}
		unsigned long int to_move = percolate_up(size,priority);
		heap[to_move].priority = priority;
		heap[to_move].item = model;
		model->q_index = to_move;
	}
	/**
	If the model is enqueued and just needs to be reschedule
	*/
	else if (enqueued && priority < DBL_MAX)
	{
		unsigned long int to_move = model->q_index;
		if (heap[to_move].priority > priority)
		{
			to_move = percolate_up(to_move,priority);
		}
		else if (heap[to_move].priority < priority)
		{
			to_move = percolate_down(to_move,priority);
		}
		else
		{
			// Don't do anything if the priority hasn't changed
			return;
		}
		heap[to_move].priority = priority;
		heap[to_move].item = model;
		model->q_index = to_move;
	}
	/**
	Otherwise, if the model is enqueued and its time of next event is infinite,
	then it needs to be removed.
	*/
	else if (enqueued)
	{
		unsigned long int to_move = model->q_index;
		T min_priority = minPriority();
		to_move = percolate_up(to_move,min_priority);
		heap[to_move].priority = min_priority;
		heap[to_move].item = model;
		model->q_index = to_move;
		removeMinimum();
	}
	/**
	Otherwise, the model is not enqueued and its passive, so don't do
	anything.
	*/
}

template <class X, class T>
void Schedule<X,T>::removeMinimum()
{
	// Don't do anything if the heap is already empty
	if (size == 0)
	{
		return;
	}
	size--;
	// Set index to 0 to indicate that this model is no longer in the schedule
	heap[1].item->q_index = 0;
	heap[1].item = NULL;
	heap[1].priority = DBL_MAX;
	if (size > 0)
	{
		heap[1] = heap[size+1];
		unsigned long int i = percolate_down(1,heap[size+1].priority);
		heap[i].item = heap[size+1].item;
		heap[i].priority = heap[size+1].priority;
		heap[i].item->q_index = i;
		heap[size+1].item = NULL;
	}
}

template <class X, class T>
void Schedule<X,T>::enlarge()
{
	heap_element* rheap = new heap_element[capacity*2];
	for (unsigned long int i = 0; i < capacity; i++)
	{
		rheap[i] = heap[i];
	}
	capacity *= 2;
	delete [] heap;
	heap = rheap;
}

template <class X, class T>
unsigned long int Schedule<X,T>::percolate_down(unsigned long int index, T priority)
{
	unsigned long int child;
	unsigned long int i = index;
	for (; i*2 <= size; i = child) 
	{
		child = i*2;
		if (child != size && heap[child+1].priority < heap[child].priority) 
		{
			child++;
		}
		if (priority > heap[child].priority) 
		{
			heap[i] = heap[child];
			heap[i].item->q_index = i;
		}
		else break;
	}
	return i;
}

template <class X, class T>
unsigned long int Schedule<X,T>::percolate_up(unsigned long int index, T priority)
{
	unsigned long int i = index;
	while (i/2 != 0 && heap[i/2].priority >= priority) 
	{
		heap[i] = heap[i/2];
		heap[i].item->q_index = i;
		i = i/2;
	}
	return i;
}

template <class X, class T>
void Schedule<X,T>::removeImminent()
{
	if (size == 0) return;
	T tN = minPriority();
	while (minPriority() <= tN)
	{
		removeMinimum();
	}
}

template <class X, class T>
void Schedule<X,T>::getImminent(Bag<Atomic<X>*>& imm, unsigned long int root) const
{
	if (root > size || heap[root].priority > heap[1].priority)
	{
		return;
	}
	heap[root].item->active = true;
	imm.insert(heap[root].item);
	getImminent(imm,root*2);
	getImminent(imm,root*2+1);
}

} // end of namespace

#endif
