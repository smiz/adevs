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
#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>

#include "adevs/models.h"
#include "adevs/time.h"

namespace adevs {

/*
 * This is a refactored version of the scheduler which uses built in types for memory
 * management and simplifies the interface. Models can only be active in a simulation
 * when added to the schedule, and they become inactive when removed. The schedule
 * is a source of truth for every model in the simulation.
 *
 * Useful examples and references:
 * - https://www.geeksforgeeks.org/custom-comparator-in-priority_queue-in-cpp-stl/
 * - https://www.geeksforgeeks.org/implement-heap-in-c/
 * - https://en.cppreference.com/w/cpp/algorithm#Heap_operations
 *
 * Possible optimizations:
 * - Heap entries that map the model and its time advance
 * - Reduce number of calls to ta() to once when a model is added or updated
 * - Only add models with valid time advances to the heap to keep it smaller.
 * - Infinite time advance models still need to be tracked.
 */
template <class OutputType, class TimeType = double>
class Schedule {
  public:
    using Model = Atomic<OutputType, TimeType>;

    // --- Model management functions ---

    void add(std::shared_ptr<Model> model) {
        // Make sure this model is not already in the schedule
        if (auto search = model_index.find(model); search != model_index.end()) {
            throw exception("Model is already active!");
        }

        // Add model to the back of the heap, set its index, then move it up the heap.
        model_heap.push_back(Entry(model));
        model_index[model] = model_heap.size() - 1;
        percolate_up(model_heap.size() - 1);
    }

    void remove(std::shared_ptr<Model> model) {
        // Find the model if it is in the schedule
        auto search = model_index.find(model);
        if (search == model_index.end()) {
            throw exception("Model is not active!");
        }

        // Remove the model from the heap and remove its index mapping
        remove_index(search->second);
        model_index.erase(search);
    }

    void update(std::shared_ptr<Model> model) {
        // Find the model if it is in the schedule
        auto search = model_index.find(model);
        if (search == model_index.end()) {
            throw exception("Model is not active!");
        }

        // Update the time advance then move the model up and down the heap.
        model_heap[search->second].time_advance = model_heap[search->second].model->ta();
        percolate_down(percolate_up(search->second));
    }

    /// @brief Rebuilds the entire schedule.
    void update_all() {
        // This is an alternative way for the simulator to update the schedule
        // by rebuild the entire heap all at once. This is likely less efficent
        // than calling update() on active models and only moving them as needed.
        // Only use this as a last ditch effort to restore the heap. All other
        // functions (add, remove, remove_next, and update) should keep the heap
        // property valid during their operations.

        // Rebuild the entire heap then loop and update each model's index
        make_heap(model_heap.begin(), model_heap.end(), compare_time_advance);
        for (size_t ii = 0; ii < model_heap.size(); ii++) {
            model_index[model_heap[ii].model] = ii;
        }
    }

    /// Remove the model at the front of the queue.
    void remove_next() {
        // Remove the index mapping first since we know the offset
        model_index.erase(model_heap.front().model);
        remove_index(0);
    }

    void check_imminent(size_t index, size_t maximum, TimeType minimum,
						std::list<std::shared_ptr<Model>> &activated) {
        if (index > maximum) {
            return;
        }

        std::shared_ptr<Model> tmp = model_heap[index];
        if (tmp.time_advance > minimum) {
            return;
        }

        assert(tmp.model->outputs->empty());
        tmp.model->imminent = true;

        // Put it in the active list if it is not already there
        if (!tmp.model->activated) {
            activated.push_back(tmp.model);
            if (tmp.model->typeIsMealyAtomic() == nullptr) {
                tmp.model->activated = true;
            }
        }

        check_imminent((index * 2) + 1, maximum, minimum, activated);
        check_imminent((index * 2) + 2, maximum, minimum, activated);
    }

    /// Gets a list of all imminent models
    std::list<std::shared_ptr<Model>> get_imminent() {
    	std::list<std::shared_ptr<Model>> activated;
        check_imminent(0, model_heap.size() - 1, get_minimum(), activated);
        return activated;
    }

    /// Get the model at the front of the queue.
    std::shared_ptr<Model> get_next() const {
        if (!model_heap.empty()) {
            //return model_heap.front().model;
            return model_heap[0].model;
        }
        return nullptr;
    }

    /// Get the time for the next event
    TimeType get_minimum() const {
        if (!model_heap.empty()) {
            return model_heap.front().time_advance;
        }
        return adevs_inf<TimeType>();
    }

    // --- General vector management ---

    /// Remove all models from the heap
    void clear() {
        model_heap.clear();
        model_index.clear();
    }

    /// Returns true if the queue is empty, and false otherwise.
    bool empty() const { return model_heap.empty(); }

    /// Get the number of elements in the heap.
    size_t size() const { return model_heap.size(); }

  private:
    struct Entry {
        Entry(std::shared_ptr<Model> m) : model(m), time_advance(m->ta()) {}
        std::shared_ptr<Model> model;
        TimeType time_advance;
    };

    // Stores all models using a heap as a priority queue
    std::vector<Entry> model_heap;
    // Stores location of all models in the heap (So they can be updated directly)
    std::unordered_map<std::shared_ptr<Model>, size_t> model_index;

    // The custom comparison operator is needed for make_heap() in update_all().
    // Compares model priorities by looking at their time advance.
    struct {
        bool operator()(Entry first, Entry second) const {
            if (first.time_advance > second.time_advance) {
                return true;
            }
            return false;
        }
    } compare_time_advance;

    // --- Heap helper functions ---
    // This assumes the rest of the tree is built using proper methods.
    // If that somehow isn't true, then use update_all(). The main spot this might
    // happen is if the simulator didn't call update() on each activated model.

    // Remove a model from the tree and restore the heap
    void remove_index(size_t index) {
        // Replace the index with the last element and then rebuild the heap.
        // Then remove the last element from the vector; don't need to keep the
        // initial element because it is being removed.
        model_heap[index] = model_heap.back();
        model_heap.pop_back();
        percolate_down(index);
    }

    // Restore the heap property by moving a model up the tree
    size_t percolate_up(size_t index) {
        size_t child = index;
        size_t parent = (index - 1) / 2;

        while (child != 0 && model_heap[child].time_advance < model_heap[parent].time_advance) {
            // Swap the models since the child's time advance is less than the parent's
        	std::swap(model_heap[child], model_heap[parent]);

            // Update the index locations of the new swapped models
            model_index[model_heap[parent].model] = parent;
            model_index[model_heap[child].model] = child;

            // Set the new parent and child indexes
            child = parent;
            parent = (child - 1) / 2;
        }
        return child;
    }

    // Restore the heap property by moving a model down the tree
    size_t percolate_down(size_t index) {

        size_t current = index;
        size_t child = (2 * current) + 1;
        size_t size = model_heap.size();

        // At most, loop until the model is at the bottom of the heap (no valid children)
        while (child < size) {

            // If the left child isn't the last model, use whichever model has a smaller time_advances
            if ((child != size - 1) &&
                model_heap[child + 1].time_advance < model_heap[child].time_advance) {
                child++;
            }

            // Sawp the models if the child's time_advance is less than the current
            if (model_heap[child].time_advance < model_heap[current].time_advance) {
                // Swap the models since the child's time advance is less than the parent's
            	std::swap(model_heap[current], model_heap[child]);

                // Update the index locations of the new swapped models
                model_index[model_heap[current].model] = current;
                model_index[model_heap[child].model] = child;

                // Update which node we are checking
                current = child;
                child = (2 * current) + 1;
            } else {
                // Heap property was satisfied
                break;
            }
        }
        return current;
    }
};

}  // namespace adevs

#endif
