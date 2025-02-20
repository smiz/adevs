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

#include <algorithm>
#include <cfloat>
#include <cstdlib>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <vector>

#include "adevs/models.h"
#include "adevs/time.h"

using namespace std;


namespace adevs {

/*
 * This is an updated version of the scheduler which uses built in types for memory
 * management and simplifies the interface. It uses a custom comparison function
 * to manage the heap by directly calling the models time_advance function. Models
 * will only be active in the simulation when they are added to the schedule and
 * should be removed from the schedule if being destroyed. The schedule will be
 * the source of truth for all active models in the simulation.

 * Possible optimizations for later:
 *  - Custom add, remove, and update functions to better optimize the queue (instead of make_heap and sort to find)
 *  - Option where models without a time advance are only stored in the list and not the heap.
 *  - Use weak pointers and garbage collect? I like forcing users to remove the model
 *  - Ignore models that have an inifinite time advance unless requested? check TA then re-insert?
 *  - This uses a lot more calls to the time_advance function that could be optimized like Jim's previous version
 */

template <class OutputType, class TimeType = double>
class Schedule {
  public:
    using Model = Atomic<OutputType, TimeType>;

    // Model management functions
    void add(shared_ptr<Model> model);
    void remove(shared_ptr<Model> model);
    void update(shared_ptr<Model> modle);
    void update_all();

    /// Remove the model at the front of the queue.
    void remove_next();

    /// Gets a list of all imminent models
    list<shared_ptr<Model>> get_imminent();

    /// Get the model at the front of the queue.
    shared_ptr<Model> get_next() const;

    /// Get the time for the next event
    TimeType get_minimum() const;

    /// Returns true if the queue is empty, and false otherwise.
    bool empty() const { return model_heap.empty(); }

    /// Get the number of elements in the heap.
    unsigned int size() const { return model_heap.size(); }

  private:
    // Define a custom comparison operator that checks each models time advance.
    // There's a good chance this is slower than the other scheduler because it
    // checks every model with a function call.
    // This was a useful example to follow:
    // https://www.geeksforgeeks.org/custom-comparator-in-priority_queue-in-cpp-stl/

    struct {
        // ! FIXME: This is likely wrong, but just getting things building
        bool operator()(shared_ptr<Model> first, shared_ptr<Model> second) const {
            if (first->ta() > second->ta()) {
                return true;
            }
            return false;
        }
    } compare_time_advance;

    void rebuild_index();

    // Stores all models using a heap as a priority queue
    vector<shared_ptr<Model>> model_heap;
    // Stores location of all models in the heap (So they can be updated directly)
    map<shared_ptr<Model>, size_t> model_index;
};


template <class OutputType, class TimeType>
void Schedule<OutputType, TimeType>::add(shared_ptr<Atomic<OutputType, TimeType>> model) {

    // Make sure this model is not already in the schedule
    if (auto search = model_index.find(model); search != model_index.end()) {
        throw exception("Model is already active!");
    }

    // Add the model to the heap then rebuild the entire index
    // TODO: This is likely not efficient, but it's easy to implement right now.
    model_heap.push_back(model);
    push_heap(model_heap.begin(), model_heap.end(), compare_time_advance);
    rebuild_index();
}


template <class OutputType, class TimeType>
void Schedule<OutputType, TimeType>::remove(shared_ptr<Atomic<OutputType, TimeType>> model) {

    // Make sure this model is not already in the schedule
    auto search = model_index.find(model);
    if (search == model_index.end()) {
        throw exception("Model is not active!");
    }

    // The heap is stored using a vector, so there is no easy way to remove a model
    // from the middle. Replace this index with a copy of the last model, pop the
    // last model from the vector, rebuild the heap, rebuild the index, and finally
    // remove the model from the index list).
    // TODO: This is likely not efficient, but it's easy to implement right now.
    model_heap[search->second] = model_heap.back();
    model_heap.pop_back();
    make_heap(model_heap.begin(), model_heap.end(), compare_time_advance);
    rebuild_index();
    model_index.erase(search);
}


template <class OutputType, class TimeType>
void Schedule<OutputType, TimeType>::update(shared_ptr<Atomic<OutputType, TimeType>> model) {
    // TODO: This is for future optimization, but it's easier to rebuild the whole thing right now.
    update_all();
}


template <class OutputType, class TimeType>
void Schedule<OutputType, TimeType>::update_all() {
    // Rebuilds the heap and then searches to update each model's index.
    make_heap(model_heap.begin(), model_heap.end(), compare_time_advance);
    rebuild_index();
}

/// Remove the model at the front of the queue.
template <class OutputType, class TimeType>
void Schedule<OutputType, TimeType>::remove_next() {
    // Same as std::priority_queue<T, Container, Compare>::pop();
    pop_heap(model_heap.begin(), model_heap.end(), compare_time_advance);
    model_index.erase(model_heap.back());
    model_heap.pop_back();
    rebuild_index();
}

template <class OutputType, class TimeType>
void Schedule<OutputType, TimeType>::rebuild_index() {
    // Rebuilds the index by looping through the entire heap and updating locations.
    for (size_t ii = 0; ii < model_heap.size(); ii++) {
        // Get the shared pointer at the index (which is the key) then save the index.
        model_index[model_heap[ii]] = ii;
    }
}

/// Get the model at the front of the queue.
template <class OutputType, class TimeType>
shared_ptr<Atomic<OutputType, TimeType>> Schedule<OutputType, TimeType>::get_next() const {
    if (!model_heap.empty()) {
        return model_heap.front();
    }
    return nullptr;
}

/// Get the time for the next event
template <class OutputType, class TimeType>
TimeType Schedule<OutputType, TimeType>::get_minimum() const {
    if (!model_heap.empty()) {
        return model_heap.front()->ta();
    }
    return adevs_inf<TimeType>();
}

template <class OutputType, class TimeType>
list<shared_ptr<Atomic<OutputType, TimeType>>> Schedule<OutputType, TimeType>::get_imminent(void) {

    // Copy the active heap, sort, and then pull the imminent items
    vector<shared_ptr<Atomic<OutputType, TimeType>>> tmp = model_heap;
    sort_heap(tmp.begin(), tmp.end(), compare_time_advance);

    list<shared_ptr<Atomic<OutputType, TimeType>>> activated;
    TimeType minimum = get_minimum();
    for (auto ii : tmp) {
        if (ii->ta() > minimum) {
            break;
        }
        activated.push_back(ii);
    }
    return activated;
}


}  // namespace adevs

#endif
