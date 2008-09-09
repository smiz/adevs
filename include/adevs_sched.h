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
		/// Creates a scheduler with the default or specified initial capacity.
		Schedule(unsigned int capacity = 100):
		capacity(capacity),
		size(0)
		{
			heap = new heap_element[capacity];
		}
		/// Get the model at the front of the queue.
		Atomic<X>* getMinimum() const { return heap[1].item; }
		/// Get the time of the next event.
		T minPriority() const { return heap[1].priority; }
		/// Get the imminent models and set their active flags to true.
		void getImminent(Bag<Atomic<X>*>& imm) const { getImminent(imm,1); }
		/**
		Add a model to the schedule or change to position of a model already in
		the schedule. If the tN is DBL_MAX, then the item is removed
		(if it is already in the queue) or not inserted.  Because this method
		uses the model's q_index field, it is imperative that a model be 
		entered into one, and only one, schedule at any time.
		*/
		void schedule(Atomic<X>* model, T priority);
		/// Remove the model at the front of the queue.
		void removeMinimum();
		/// Remove the imminent models from the queue.
		void removeImminent();
		/// Returns true if the queue is empty, and false otherwise.
		bool empty() const { return size == 0; }
		/**
		 * Get the model at position k in the heap. The value of k must be between
		 * 1 and size inclusive.
		 */
		Atomic<X>* get(unsigned int k) const { return heap[k].item; }
		/// Get the number of elements in the heap.
		unsigned int getSize() const { return size; }
		/// Destructor.
		~Schedule() { delete [] heap; }
	private:
		// Definition of an element in the heap.
		struct heap_element 
		{
			Atomic<X>* item;
			T priority;
			// Constructor initializes the item and priority
			heap_element():
				item(NULL),priority(DBL_MAX){}
		};
		heap_element* heap;
		unsigned int capacity, size;

		/// Double the schedule capacity
		void enlarge();
		/**
		 * Heap percolation functions.  Returns the empty slot that emerges for
		 * a new item with the provided priority.
		 */
		unsigned int percolate_down(unsigned int index, T priority);
		unsigned int percolate_up(unsigned int index, T priority);
		/// Construct the imminent set recursively
		void getImminent(Bag<Atomic<X>*>& imm, unsigned int root) const;
};

template <class X,class T>
void Schedule<X,T>::schedule(Atomic<X>* model, T priority)
{
	// Check for existance in the queue. 
	bool enqueued = false;
	if (model->q_index != 0)
		enqueued = (heap[model->q_index].item == model); 
	// If the item must be added to the schedule
	if (!enqueued && priority < DBL_MAX)
	{
		// Increment the size of the heap and make sure there is space
		size++;
		if (size == capacity) enlarge();
		// Find the right slot and put the item into it
		unsigned int to_move = percolate_up(size,priority);
		heap[to_move].priority = priority;
		heap[to_move].item = model;
		model->q_index = to_move;
	}
	// If the model is already enqueued and needs to be rescheduled
	else if (enqueued && priority < DBL_MAX)
	{
		unsigned int to_move = model->q_index;
		// Decrease the time to next event
		if (heap[to_move].priority > priority)
			to_move = percolate_up(to_move,priority);
		// Increase the time to next event
		else if (heap[to_move].priority < priority)
			to_move = percolate_down(to_move,priority);
		// Don't do anything if the priority hasn't changed
		else return;
		heap[to_move].priority = priority;
		heap[to_move].item = model;
		model->q_index = to_move;
	}
	// If the model is enqueued and it must be removed
	else if (enqueued)
	{
		unsigned int to_move = model->q_index;
		// Move the item to the top of the heap
		T min_priority = minPriority();
		to_move = percolate_up(to_move,min_priority);
		heap[to_move].priority = min_priority;
		heap[to_move].item = model;
		model->q_index = to_move;
		// Remove it
		removeMinimum();
	}
	// Otherwise, the model is not enqueued and has no next event
}

template <class X, class T>
void Schedule<X,T>::removeMinimum()
{
	// Don't do anything if the heap is already empty
	if (size == 0) return;
	size--;
	// Set index to 0 to indicate that this model is no longer in the schedule
	heap[1].item->q_index = 0;
	heap[1].item = NULL;
	heap[1].priority = DBL_MAX;
	if (size > 0)
	{
		heap[1] = heap[size+1];
		unsigned int i = percolate_down(1,heap[size+1].priority);
		heap[i] = heap[size+1];
		heap[i].item->q_index = i;
		heap[size+1].item = NULL;
	}
}

template <class X, class T>
void Schedule<X,T>::enlarge()
{
	heap_element* rheap = new heap_element[capacity*2];
	for (unsigned int i = 0; i < capacity; i++)
		rheap[i] = heap[i];
	capacity *= 2;
	delete [] heap;
	heap = rheap;
}

template <class X, class T>
unsigned int Schedule<X,T>::percolate_down(unsigned int index, T priority)
{
	unsigned int child, i = index;
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
unsigned int Schedule<X,T>::percolate_up(unsigned int index, T priority)
{
	unsigned int i = index;
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
		removeMinimum();
}

template <class X, class T>
void Schedule<X,T>::getImminent(Bag<Atomic<X>*>& imm, unsigned int root) const
{
	if (root > size || heap[root].priority > heap[1].priority)
		return;
	heap[root].item->active = true;
	imm.insert(heap[root].item);
	getImminent(imm,root*2);
	getImminent(imm,root*2+1);
}

} // end of namespace

#endif
