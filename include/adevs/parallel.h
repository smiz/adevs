
/*
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
 * ANY EValueTypePRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EValueTypeEMPLARY, OR CONSEQUENTIAL DAMAGES
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

#ifndef _adevs_parallel_simulator_h_
#define _adevs_parallel_simulator_h_

#include <any>
#include <cassert>
#include <list>
#include <vector>
#include <memory>
#include "adevs/graph.h"
#include "adevs/models.h"
#include "adevs/time.h"

namespace adevs {

template <class ValueType = std::any, class TimeType = adevs::sd_time<>>
class ParallelSimulator {

  public:
    /** 
     * @brief Create a simulator for an atomic model.
     * 
     * The constructor will fail and throw an adevs::exception if the
     * time advance of the model is less than zero.
     * @param model The model to simulate.
     */
    ParallelSimulator(std::shared_ptr<Atomic<ValueType, TimeType>> model);

    /**
     * @brief Initialize the simulator with a collection of models.
     *
     * The constructor will fail and throw an adevs::exception if the
     * time advance of any model is less than zero.
     * @param model The graph to simulate.
     */
    ParallelSimulator(std::shared_ptr<Graph<ValueType, TimeType>> model);

    /**
     * @brief Initialize the simulator with a Coupled model.
     * 
     * @param model The Coupled model to simulate.
     */
    ParallelSimulator(std::shared_ptr<Coupled<ValueType, TimeType>> model);

    /**
     * @brief Destructor leaves models intact.
     * 
     */
    ~ParallelSimulator() {
        for (auto lp: lps) {
            // Cause any input and output objects to be destroyed
            lp->input.clear();
            lp->output.clear();
            // Cleanup the checkpoint objects
            for (auto checkpoint: lp->checkpoints) {
                lp->model->destroy_checkpoint(checkpoint.saved_state);
            }
            delete lp;
        }
    }

    /**
     * @brief Simulate the model
     * 
     * This executes the simulation until the global virtual time
     * exceeds the given termination time.
     * 
     * @param t_end Termination time for the simulation
     */
    void exec_until(TimeType t_end);

  private:
    std::shared_ptr<Graph<ValueType, TimeType>> graph;
    TimeType gvt; // global virtual time
    TimeType t_end; // Simulation stop time

    struct lp_t;
    struct event_t {
        lp_t* src; // logical process that created the event
        lp_t* dst; // logical process that received the event
        TimeType time_stamp; // time of the event
        PinValue<ValueType> value; // Value of the event
    };
    struct checkpoint_t {
        void* saved_state; // The saved state
        TimeType time_stamp; // Time at which the state was valid
    };

    /// An event exchanged between logical processes
    /// A logical process that wraps an atomic model and
    /// provides support for its speculative execution
    struct lp_t {
        std::shared_ptr<Atomic<ValueType,TimeType> > model;
        bool compute_output;
        std::list<event_t> input, output;
        std::list<checkpoint_t> checkpoints;

        lp_t(std::shared_ptr<Atomic<ValueType,TimeType>>& model):
            model(model),compute_output(true){}
    };

    std::vector<lp_t*> lps;

    void output_and_gvt();
    void state_change_and_garbage_collect();
    void garbage_collect(lp_t* lp);
    void rollback(lp_t* lp);

    static void calculate_tN(std::shared_ptr<Atomic<ValueType,TimeType>>& atomic);
    static void insert_into_list(event_t& msg, std::list<event_t>& dst);
    static void remove_from_list(lp_t* src, TimeType time_stamp, std::list<event_t>& input);
    void create_lp(std::shared_ptr<Atomic<ValueType,TimeType>>& atomic);
    void checkpoint(lp_t* lp);
};

template <typename ValueType, typename TimeType>
void ParallelSimulator<ValueType, TimeType>::checkpoint(lp_t* lp) {
    checkpoint_t checkpoint;
    checkpoint.time_stamp = lp->model->tL;
    checkpoint.saved_state = lp->model->make_checkpoint();
    lp->checkpoints.push_back(checkpoint);
}

template <typename ValueType, typename TimeType>
void ParallelSimulator<ValueType, TimeType>::calculate_tN(std::shared_ptr<Atomic<ValueType, TimeType>>& atomic) {
    TimeType time_advance(atomic->ta());
    if (time_advance < adevs_inf<TimeType>()) {
        atomic->tN = time_advance;
    } else {
        atomic->tN = atomic->tL + time_advance;
    }
}

template <typename ValueType, typename TimeType>
void ParallelSimulator<ValueType, TimeType>::create_lp(std::shared_ptr<Atomic<ValueType, TimeType>>& atomic) {
    atomic->q_index = lps.size();
    atomic->tL = adevs_zero<TimeType>();
    calculate_tN(atomic);
    lps.push_back(new lp_t(atomic));
}


template <typename ValueType, typename TimeType>
ParallelSimulator<ValueType, TimeType>::ParallelSimulator(std::shared_ptr<Graph<ValueType, TimeType>> model)
    : graph(model) {
    graph->set_provisional(true);
    for (auto atomic : model->get_atomics()) {
        create_lp(atomic);
    }
}

template <typename ValueType, typename TimeType>
ParallelSimulator<ValueType, TimeType>::ParallelSimulator(std::shared_ptr<Atomic<ValueType, TimeType>> model)
    : graph(new Graph<ValueType, TimeType>()) {
    graph->add_atomic(model);
    graph->set_provisional(true);
    create_lp(model);
}

template <typename ValueType, typename TimeType>
ParallelSimulator<ValueType, TimeType>::ParallelSimulator(std::shared_ptr<Coupled<ValueType, TimeType>> model)
    : graph(new Graph<ValueType, TimeType>()) {
    model->assign_to_graph(graph.get());
    graph->set_provisional(true);
    for (auto atomic : graph->get_atomics()) {
        create_lp(atomic);
    }
}

template <typename ValueType, typename TimeType>
void ParallelSimulator<ValueType, TimeType>::exec_until(TimeType t_end) {
    this->t_end = t_end;
    do {
        output_and_gvt();
        state_change_and_garbage_collect();
    } while (gvt < t_end);
}

template <typename ValueType, typename TimeType>
void ParallelSimulator<ValueType, TimeType>::insert_into_list(event_t& msg, std::list<event_t>& dst) {
    auto pos = dst.begin();
    for (; pos != dst.end(); pos++) {
        if (msg.time_stamp < (*pos).time_stamp) {
            dst.insert(pos,msg);
            return;
        }
    }
    dst.push_back(msg);
}

template <typename ValueType, typename TimeType>
void ParallelSimulator<ValueType, TimeType>::output_and_gvt() {
    gvt = adevs_inf<TimeType>();
    #pragma omp parallel for 
    for (unsigned i = 0; i < lps.size(); i++) {
        if (lps[i]->model->tN < t_end && lps[i]->compute_output) {
            event_t msg;
            std::list<std::pair<adevs::pin_t,std::shared_ptr<Atomic<ValueType,TimeType>>>> input;
            msg.time_stamp = lps[i]->model->tN;
            msg.src = lps[i];
            lps[i]->model->output_func(lps[i]->model->outputs);
            for (auto y : lps[i]->model->outputs) {
                msg.value = y;
                graph->route(y.pin, input);
                for (auto consumer : input) {
                    msg.dst = lps[consumer.second->q_index];
                    msg.value.pin = consumer.first;
                    insert_into_list(msg,msg.dst->input);
                    insert_into_list(msg,msg.src->output);
                }
                input.clear();
            }
            lps[i]->model->outputs.clear();
            lps[i]->compute_output = false;
        }
        gvt = std::min(gvt,lps[i]->model->tN);
    }
}

template <typename ValueType, typename TimeType>
void ParallelSimulator<ValueType, TimeType>::remove_from_list(lp_t* src, TimeType time_stamp, std::list<event_t>& input) {
    auto pos = input.begin();
    for (; pos != input.end(); pos++) {
        if ((*pos).src == src && (*pos).time_stamp == time_stamp) {
            input.erase(pos);
            return;
        }
    }
}

template <typename ValueType, typename TimeType>
void ParallelSimulator<ValueType, TimeType>::garbage_collect(lp_t* lp) {
    while (!lp->output.empty() && lp->output.front().time_stamp < gvt) {
        lp->output.pop_front();
    }
    auto cur = lp->checkpoints.begin();
    if (cur == lp->checkpoints.end()) {
        return;
    }
    auto next = cur;
    next++;
    while ((*cur).time_stamp < gvt && next != lp->checkpoints.end() && (*next).time_stamp < gvt) {
        std::cout << "deleting " << (*cur).time_stamp << " , gvt = " << gvt << std::endl;
        lp->model->destroy_checkpoint((*cur).saved_state);
        cur = lp->checkpoints.erase(cur);
        next = cur;
        next++;
    }
    assert(!lp->checkpoints.empty());
    assert(lp->checkpoints.front().time_stamp <= gvt);
}

template <typename ValueType, typename TimeType>
void ParallelSimulator<ValueType, TimeType>::rollback(lp_t* lp) {
    while (lp->checkpoints.back().time_stamp > gvt) {
        lp->model->destroy_checkpoint(lp->checkpoints.back().saved_state);
        lp->checkpoints.pop_back();
    }
    assert(!lp->checkpoints.empty());
    lp->model->restore_checkpoint(lp->checkpoints.back().saved_state);
    lp->model->tL = lp->checkpoints.back().time_stamp;
    calculate_tN(lp->model);
    while (!lp->output.empty() && lp->output.back().time_stamp > gvt) {
        remove_from_list(lp,lp->output.back().time_stamp,lp->output.back().dst->input);
        lp->output.pop_back();
    }
}

template <typename ValueType, typename TimeType>
void ParallelSimulator<ValueType, TimeType>::state_change_and_garbage_collect() {
    #pragma omp parallel for
    for (unsigned i = 0; i < lps.size(); i++) {
        garbage_collect(lps[i]);
        // Roll back if needed
        TimeType next_input_time((lps[i]->input.empty()) ? adevs_inf<TimeType>() : lps[i]->input.front().time_stamp);
        // Restore the checkpoint if we have input in the to process. Create a checkpoint if
        // we will change state but have not restored a checkpoint.
        if (next_input_time == gvt && gvt < lps[i]->model->tL) {
            rollback(lps[i]);
        } else if (next_input_time == gvt || lps[i]->model->tN < adevs_inf<TimeType>()) {
            std::cout << "lp = " << i << " " << lps[i]->model->tL << std::endl;
            checkpoint_t checkpoint;
            checkpoint.saved_state = lps[i]->model->make_checkpoint();
            checkpoint.time_stamp = lps[i]->model->tL;
            lps[i]->checkpoints.push_back(checkpoint);
        }
        // Gather input at GVT
        while (!lps[i]->input.empty() && lps[i]->input.front().time_stamp == gvt) {
            lps[i]->model->inputs.push_back(lps[i]->input.front().value);
            lps[i]->input.pop_front();
        }
        if (!lps[i]->model->inputs.empty() && gvt < lps[i]->model->tN) {
            lps[i]->model->delta_ext(next_input_time - lps[i]->model->tL, lps[i]->model->inputs);
            lps[i]->model->inputs.clear();
            lps[i]->model->tL = next_input_time + adevs_epsilon<TimeType>();
            calculate_tN(lps[i]->model);
            lps[i]->compute_output = true;
        } else if (!lps[i]->model->inputs.empty() && gvt == lps[i]->model->tN) {
            lps[i]->model->delta_conf(lps[i]->model->inputs);
            lps[i]->model->inputs.clear();
            lps[i]->model->tL = next_input_time + adevs_epsilon<TimeType>();
            calculate_tN(lps[i]->model);
            lps[i]->compute_output = true;
        } else if (lps[i]->model->inputs.empty() && lps[i]->model->tN < adevs_inf<TimeType>()) {
            lps[i]->model->delta_int();
            lps[i]->model->tL = lps[i]->model->tN + adevs_epsilon<TimeType>();
            calculate_tN(lps[i]->model);
            lps[i]->compute_output = true;
        }
    }
}

}  // namespace adevs

#endif
