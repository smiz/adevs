#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include "adevs/adevs.h"

/**
 * This test compares a randomly generated discrete 
 * time CA with its discrete event equivalent.
 */
 
 #define NUM_SYMBOLS 3

 class Rule {
    public:
    Rule() {
        for (int i = 0 ;i < NUM_SYMBOLS; i++) {
            for (int j = 0; j < NUM_SYMBOLS; j++) {
                dq[i][j] = rand()%NUM_SYMBOLS;
                y[i][j] = rand()%NUM_SYMBOLS;
            }
        }
    }
    int next_state(int q, int x) const {
        return dq[q][x];
    }
    int output(int q, int x) const {
        return y[q][x];
    }
    private:
    int dq[2][2];
    int y[2][2];
 };

#define NUM_CELLS 16

struct event_t {
    int src, value;
};

using Bag = std::list<adevs::PinValue<event_t>>;

static int external_cell = 0;
static int confluent_cell = 0;
static int imm_cell = 0;
static int max_revisions = 0;

class Cell: public adevs::MealyAtomic<event_t,int> {
    public:
    Cell(int pos, const Rule& rule, int q0, int x0[2], int y0):
    adevs::MealyAtomic<event_t,int>(),
    pos(pos),q(q0),y(y0),rule(rule),revisions(-1) {
        x[0] = x0[0];
        x[1] = x0[1];
    }

    int ta() {
        if (rule.next_state(q,input()) == q && rule.output(q,input()) == y) {
            return adevs_inf<int>();
        }
        return 1;
    }

    void delta_int() {
        if (revisions >= 0) {
            max_revisions = std::max(revisions,max_revisions);
        }
        revisions = -1;
        y = rule.output(q,input());
        q = rule.next_state(q,input());
    }
    void delta_ext(int, const Bag& xb) {
        if (revisions >= 0) {
            max_revisions = std::max(revisions,max_revisions);
        }
        revisions = -1;
        for (auto event: xb) {
            int idx = (pos-1) - event.value.src;
            x[idx] = event.value.value;
        }
        y = rule.output(q,input());
        q = rule.next_state(q,input());
    }
    void delta_conf(const Bag& xb) {
        delta_ext(0,xb);
    }
    void output_func(Bag& yb) {
        revisions++;
        imm_cell++;
        event_t event;
        event.src = pos;
        event.value = rule.output(q,input());
        yb.push_back(adevs::PinValue<event_t>(output,event));
    }
    void external_output_func(int e, Bag const &xb, Bag& yb) {
        revisions++;
        external_cell++; 
        confluent_cell--;
        confluent_output_func(xb,yb);
    }
    void confluent_output_func(Bag const &xb, Bag& yb) {
        revisions++;
        confluent_cell++;
        int tmp[2];
        memcpy(tmp,x,sizeof(int)*2);
        for (auto event: xb) {
            int idx = (pos-1) - event.value.src;
            x[idx] = event.value.value;
        }
        event_t event;
        event.src = pos;
        event.value = rule.output(q,input());
        yb.push_back(adevs::PinValue<event_t>(output,event));
        memcpy(x,tmp,sizeof(int)*2);
    }

    const adevs::pin_t output;

    int get_state() const { return q; }
    int get_output() const { return y; }
    int get_input() const { return input(); }

    private:
    const int pos;
    int q, x[2], y;
    const Rule& rule;
    int revisions;

    int input() const {
        return (x[0] + x[1]) % NUM_SYMBOLS;
    }
};

struct step_t {
    int q[NUM_CELLS];
    int x[NUM_CELLS];
    int y[NUM_CELLS];

    bool operator==(const step_t other) const {
        for (int i = 0; i < NUM_CELLS; i++) {
            if (q[i] != other.q[i]) return false;
            if (x[i] != other.x[i]) return false;
            if (y[i] != other.y[i]) return false;
        }
        return true;
    }
};

std::list<step_t> discrete_time(const Rule& rule, const int q0[NUM_CELLS], const int num_steps) {
    std::list<step_t> traj;
    step_t s;
    memcpy(s.q,q0,NUM_CELLS*sizeof(int));
    for (int i = 0; i < num_steps; i++) {
        for (int j = 0; j < NUM_CELLS; j++) {
            switch (j) {
                case 0:
                    s.x[j] = 0;
                    break;
                case 1:
                    s.x[j] = (0 + s.y[j-1]) % NUM_SYMBOLS;
                    break;
                default:
                    s.x[j] = (s.y[j-1] + s.y[j-2]) % NUM_SYMBOLS;
            }
            s.y[j] = rule.output(s.q[j],s.x[j]);
        }
        for (int j = 0; j < NUM_CELLS; j++) {
            s.q[j] = rule.next_state(s.q[j],s.x[j]);
        }
        traj.push_back(s);
    }
    return traj;
}

std::list<step_t> discrete_event(const Rule& rule, const int q0[NUM_CELLS], const int num_steps) {
    std::list<step_t> traj;
    step_t s;
    std::shared_ptr<Cell> cells[NUM_CELLS];
    auto graph = std::make_shared<adevs::Graph<event_t,int>>();
    memcpy(s.q,q0,NUM_CELLS*sizeof(int));
    /// Get initial conditions
    for (int j = 0; j < NUM_CELLS; j++) {
        int x[2] = { 0, 0 };
        switch (j) {
            case 0:
                s.x[j] = 0;
                break;
            case 1:
                s.x[j] = (0 + s.y[j-1]) % NUM_SYMBOLS;
                x[0] = s.y[j-1];
                break;
            default:
                s.x[j] = (s.y[j-1] + s.y[j-2]) % NUM_SYMBOLS;
                x[0] = s.y[j-1];
                x[1] = s.y[j-2];
        }
        s.y[j] = rule.output(s.q[j],s.x[j]);
        auto cell = std::make_shared<Cell>(j,rule,s.q[j],x,s.y[j]);
        cells[j] = cell;
        graph->add_atomic(cell);
        switch (j) {
            case 0:
                break;
            case 1:
                graph->connect(cells[j-1]->output,cells[j]);
                break;
            default:
                graph->connect(cells[j-1]->output,cells[j]);
                graph->connect(cells[j-2]->output,cells[j]);
                break;
        }
    }
    auto sim = std::make_shared<adevs::Simulator<event_t,int>>(graph);
    for (int i = 0; i < num_steps; i++) {
        if (sim->nextEventTime()) {
            sim->execNextEvent();
            for (int j = 0; j < NUM_CELLS; j++) {
                s.q[j] = cells[j]->get_state();
                s.x[j] = cells[j]->get_input();
                s.y[j] = cells[j]->get_output();
            }
            traj.push_back(s);
        } else {
            traj.push_back(s);
        }
    } 
    return traj;
}

void print(const std::list<step_t>& traj) {
    for (auto step: traj) {
        std::cout << "q: ";
        for (int i = 0; i < NUM_CELLS; i++) {
            std::cout << step.q[i];
        }
        std::cout << '\n';
        std::cout << "x: ";
        for (int i = 0; i < NUM_CELLS; i++) {
            std::cout << step.x[i];
        }
        std::cout << '\n';
        std::cout << "y: ";
        for (int i = 0; i < NUM_CELLS; i++) {
            std::cout << step.y[i];
        }
        std::cout << '\n';
    }
}

int main() {
    const int num_steps = 20;
    const int num_trials = 200;
    for (int trial = 0; trial < num_trials; trial++) {
        const Rule rule;
        int q0[NUM_CELLS];
        for (int i = 0; i < NUM_CELLS; i++) {
            q0[i] = rand()%NUM_SYMBOLS;
        }
        std::list<step_t> traj_dt(discrete_time(rule,q0,num_steps));
        std::list<step_t> traj_de(discrete_event(rule,q0,num_steps));
        std::cout << "DT trial #" << trial << '\n';
        print(traj_dt);
        std::cout << "DE trial #" << trial << '\n';
        print(traj_de);
        assert(traj_dt.size() == traj_de.size());
        auto iter_dt = traj_dt.begin();
        auto iter_de = traj_de.begin();
        while (iter_dt != traj_dt.end()) {
            assert((*iter_dt) == (*iter_de));
            iter_dt++;
            iter_de++;
        }
    }
    // Some statistics
    std::cout << "external output calls = " << external_cell << '\n';
    std::cout << "confluent output calls = " << confluent_cell << '\n';
    std::cout << "imm output calls = " << imm_cell << '\n';
    std::cout << "max # revisions = " << max_revisions << '\n';
    return 0;
}