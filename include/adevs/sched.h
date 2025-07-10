/*
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

#ifndef _adevs_schedule_h_
#define _adevs_schedule_h_

#include <cassert>
#include <cfloat>
#include <cstdlib>
#include <list>
#include <memory>
#include "adevs/models.h"
#include "adevs/time.h"

using namespace std;


namespace adevs {

/*
 * This is a binary heap for scheduling Atomic models. The Schedule uses
 * the q_index attribute of the Atomic model to keep track of the object's
 * position in the heap. Therefore, a model can be put into only one Schedule.
 * Please observe that the q_index value for a model must be initialized
 * to zero before it is placed into the heap for the first time.
 */
template <class ValueType, class TimeType = double>
class Schedule {
  public:
    /// Creates a scheduler with the default or specified initial capacity.
    Schedule(unsigned int capacity = 100)
        : capacity(capacity), size(0), heap(new heap_element[capacity]) {
        heap[0].priority = adevs_sentinel<TimeType>();  // This is a sentinel value
    }
    /// Get the model at the front of the queue.
    Atomic<ValueType, TimeType>* getMinimum() const { return heap[1].item; }
    /// Get the time of the next event.
    TimeType minPriority() const { return heap[1].priority; }
    /// Visit the imminent models.
    list<Atomic<ValueType, TimeType>*> visitImminent(void) {
        activated.clear();
        visitImminent(1);
        return activated;
    }
    /// Remove the model at the front of the queue.
    void removeMinimum();
    /// Add, remove, or move a model as required by its priority.
    void schedule(Atomic<ValueType, TimeType>* model, TimeType priority);
    /// Returns true if the queue is empty, and false otherwise.
    bool empty() const { return size == 0; }
    /// Get the number of elements in the heap.
    unsigned int getSize() const { return size; }
    /// Destructor.
    ~Schedule() { delete[] heap; }

  private:
    // Definition of an element in the heap.
    struct heap_element {
        Atomic<ValueType, TimeType>* item;
        TimeType priority;
        // Constructor initializes the item and priority
        heap_element() : item(nullptr), priority(adevs_inf<TimeType>()) {}
    };
    unsigned int capacity, size;
    heap_element* heap;

    list<Atomic<ValueType, TimeType>*> activated;

    /// Double the schedule capacity
    void enlarge();
    /// Move the item at index down and return its new position
    unsigned int percolate_down(unsigned int index, TimeType priority);
    /// Move the item at index up and return its new position
    unsigned int percolate_up(unsigned int index, TimeType priority);
    /// Visit the imminent set recursively
    // void visitImminent(ImminentVisitor* visitor, unsigned int root) const;
    void visitImminent(unsigned int root);

    void visit(Atomic<ValueType, TimeType>* model);
};

template <class ValueType, class TimeType>
void Schedule<ValueType, TimeType>::visit(Atomic<ValueType, TimeType> *model) {
    assert(model->outputs.empty());
    activated.push_back(model);
}

template <class ValueType, class TimeType>
void Schedule<ValueType, TimeType>::visitImminent(unsigned int root) {
    // Stop if the bottom is reached or the next priority is not equal to the minimum
    if (root > size || heap[1].priority < heap[root].priority) {
        return;
    }

    visit(heap[root].item);

    // Look for more imminent models in the left sub-tree
    visitImminent(root * 2);
    // Look in the right sub-tree
    visitImminent(root * 2 + 1);
}

template <class ValueType, class TimeType>
void Schedule<ValueType, TimeType>::removeMinimum() {
    // Don't do anything if the heap is empty
    if (size == 0) {
        return;
    }
    size--;
    // Set index to 0 to show that this model is not in the schedule
    heap[1].item->q_index = 0;
    heap[1].item = nullptr;
    // If the schedule is empty, set the priority of the last element to adevs_inf
    if (size == 0) {
        heap[1].priority = adevs_inf<TimeType>();
    } else {
        // Otherwise fill the hole left by the deleted model
        unsigned int i = percolate_down(1, heap[size + 1].priority);
        heap[i] = heap[size + 1];
        heap[i].item->q_index = i;
        heap[size+1].item = nullptr;
    }
}

template <class ValueType, class TimeType>
void Schedule<ValueType, TimeType>::schedule(Atomic<ValueType, TimeType>* model,
                                              TimeType priority) {
    // If the model is in the schedule
    if (model->q_index != 0) {
        // Remove the model if the next event time is infinite
        if (!(priority < adevs_inf<TimeType>())) {
            // Move the item to the top of the heap
            TimeType min_priority = minPriority();
            model->q_index = percolate_up(model->q_index, min_priority);
            heap[model->q_index].priority = min_priority;
            heap[model->q_index].item = model;
            // Remove it and return
            removeMinimum();
            return;
        }
        // Decrease the time to next event
        else if (priority < heap[model->q_index].priority) {
            model->q_index = percolate_up(model->q_index, priority);
        }
        // Increase the time to next event
        else if (heap[model->q_index].priority < priority) {
            model->q_index = percolate_down(model->q_index, priority);
        }
        // Don't do anything if the priority is unchanged
        else {
            return;
        }
        heap[model->q_index].priority = priority;
        heap[model->q_index].item = model;
    }
    // If it is not in the schedule and the next event time is
    // not at infinity, then add it to the schedule
    else if (priority < adevs_inf<TimeType>()) {
        // Enlarge the heap to hold the new model
        size++;
        if (size == capacity) {
            enlarge();
        }
        // Find a slot and put the item into it
        model->q_index = percolate_up(size, priority);
        heap[model->q_index].priority = priority;
        heap[model->q_index].item = model;
    }
    // Otherwise, the model is not enqueued and has no next event
}

template <class ValueType, class TimeType>
unsigned int Schedule<ValueType, TimeType>::percolate_down(unsigned int index, TimeType priority) {
    unsigned int child;
    for (; index * 2 <= size; index = child) {
        child = index * 2;
        if (child != size && heap[child + 1].priority < heap[child].priority) {
            child++;
        }
        if (heap[child].priority < priority) {
            heap[index] = heap[child];
            heap[index].item->q_index = index;
        } else {
            break;
        }
    }
    return index;
}

template <class ValueType, class TimeType>
unsigned int Schedule<ValueType, TimeType>::percolate_up(unsigned int index, TimeType priority) {
    // Position 0 has priority -1 and this method is always called
    // with priority >= 0 and index > 0.
    while (priority <= heap[index / 2].priority) {
        heap[index] = heap[index / 2];
        heap[index].item->q_index = index;
        index /= 2;
    }
    return index;
}

template <class ValueType, class TimeType>
void Schedule<ValueType, TimeType>::enlarge() {
    heap_element* rheap = new heap_element[capacity * 2];
    for (unsigned int i = 0; i < capacity; i++) {
        rheap[i] = heap[i];
    }
    capacity *= 2;
    delete[] heap;
    heap = rheap;
}

}  // namespace adevs

#endif
