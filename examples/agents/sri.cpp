/**
 * This calculates an "agent based" solution to the SRI equations.
 */
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <cmath>
#include <iostream>
#include <set>
#include <vector>
#include "adevs/adevs.h"

using namespace std;
using namespace adevs;

// Total number of agents in the population
static int num_agents = 1000;
// Fraction of the population that begins as infectious
static double const init_sick = 0.01;
// Encounter rate (contacts initiated by each agent per day)
// This is a Poisson process with a mean of three events per
// day.
static double const r = 3.0;
// Number of days that an infectious person stays infectious
// before transitioning to recovered. Once again, a Poisson
// process.
static double const d = 5.0;
// Probabiliy of a sick person making another person sick upon
// contact
static double const p = 0.1;
// Our random number generator
static gsl_rng* rnd = gsl_rng_alloc(gsl_rng_default);
// The number of agents in the S, I, and R compartments
static int const S = 0;
static int const I = 1;
static int const R = 2;
static int pop[3] = {0, 0, 0};

// A single agent in the model
class Agent : public Atomic<int> {
  public:
    Agent(int c0)
        : Atomic<int>(),
          c(c0),                    // Initial state
          td(adevs_inf<double>()),  // time to recover
          tr(adevs_inf<double>())   // time to next contact
    {
        // If we started sick
        if (c == I) {
            // How long to recover?
            td = gsl_ran_exponential(rnd, d);
            // When is our first contact?
            tr = gsl_ran_exponential(rnd, 1.0 / r);
        }
        // Update the population for our compartment
        pop[c]++;
    }
    void delta_int() {
        // A healthy person never gets here
        assert(c != S);
        // If we have recovered
        if (ta() == td) {
            // Must have been infectious
            assert(c == I);
            // Change from I to R
            pop[c]--;
            c = R;
            pop[c]++;
            // No more events for this agent!
            td = tr = adevs_inf<double>();
        }
        // Otherwise, schedule our next contact
        else if (ta() < td) {
            td -= ta();  // Decrement time to recover by elapsed time
            tr = gsl_ran_exponential(rnd, 1.0 / r);
        }
    }
    void delta_ext(double e, Bag<int> const &) {
        // If we are susceptible and get unlucky
        if (c == S && gsl_rng_uniform(rnd) < p) {
            // Switch from S to I
            pop[c]--;
            c = I;
            pop[c]++;
            // When will we get better?
            td = gsl_ran_exponential(rnd, d);
            // When do we make our first contact?
            tr = gsl_ran_exponential(rnd, 1.0 / r);
        }
        // Otherwise, just bide our time until the
        // next event (get better or contact)
        else if (c == I) {
            td -= e;
            tr -= e;
        }
    }
    void delta_conf(Bag<int> const &xb) {
        delta_int();
        delta_ext(0.0, xb);
    }
    void output_func(Bag<int> &yb) {
        // If this is our next contact, then
        // send an event to some other agent
        // that will be selected at random
        if (tr == ta()) {
            yb.push_back(c);
        }
    }

    // How long until our next event?
    double ta() { return ::min(tr, td); }

  private:
    int c;
    double td, tr;
};

/**
  * This is an Euler's method solver for the continuous
  * SIR equations. This is the "exact" solution for a
  * sufficiently large population of agents.
  */
class SIR : public Atomic<int> {
  public:
    SIR()
        : Atomic<int>(), ss(1.0 - init_sick), ii(init_sick), rr(0.0), h(0.01) {}
    double ta() { return h; }
    void delta_int() { update(h); }
    void delta_ext(double e, Bag<int> const &) { update(e); }
    void delta_conf(Bag<int> const &) { update(h); }
    void output_func(Bag<int> &) {}

    double get_s() const { return ss; }
    double get_i() const { return ii; }
    double get_r() const { return rr; }

  private:
    double ss, ii, rr;
    double const h;

    // Method does the numerical integration step
    void update(double hh) {
        double di = p * r * ss * ii - ii / d;
        double ds = -p * r * ss * ii;
        double dr = ii / d;
        ss += hh * ds;
        ii += hh * di;
        rr += hh * dr;
    }
};

class RandomNetwork : public Network<int> {
  public:
    RandomNetwork() : Network<int>() {
        // Create our population
        for (int i = 0; i < num_agents; i++) {
            if (i < init_sick * num_agents) {
                pop.push_back(new Agent(I));
            } else {
                pop.push_back(new Agent(S));
            }
            pop.back()->setParent(this);
        }
        // Evolve the ODE solution with it
        sir = new SIR();
        sir->setParent(this);
    }
    double get_s() const { return sir->get_s(); }
    double get_i() const { return sir->get_i(); }
    double get_r() const { return sir->get_r(); }
    void getComponents(set<Devs<int>*> &c) {
        for (auto agent : pop) {
            c.insert(agent);
        }
        c.insert(sir);
    }
    void route(int const &value, Devs<int>* model, Bag<Event<int>> &r) {
        // This implements our random contact network
        Event<int> xx;
        xx.value = value;
        do {
            xx.model = pop[gsl_rng_uniform_int(rnd, pop.size())];
        } while (xx.model == model);
        r.push_back(xx);
        xx.model = sir;
        r.push_back(xx);
    }
    ~RandomNetwork() {
        for (auto agent : pop) {
            delete agent;
        }
    }

  private:
    vector<Agent*> pop;
    SIR* sir;
};

void print(double t, RandomNetwork* world) {
    // Print the compartments each day
    static int next_day = -1;
    if (t > next_day) {
        next_day = (int)t;
        cout << next_day << " " <<
            // Agent model as fraction of population
            ((double)pop[S] / (double)num_agents) << " "
             << ((double)pop[I] / (double)num_agents) << " "
             << ((double)pop[R] / (double)num_agents) << " " <<
            // ODE model as fraction of population
            world->get_s() << " " << world->get_i() << " " << world->get_r()
             << " " << endl;
        next_day++;
    }
}

int main(int argc, char** argv) {
    num_agents = atoi(argv[1]);
    gsl_rng_set(rnd, atoi(argv[2]));
    RandomNetwork* world = new RandomNetwork();
    Simulator<int>* sim = new Simulator<int>(world);
    print(0.0, world);
    while (sim->nextEventTime() < 200.0) {
        double t = sim->nextEventTime();
        print(t, world);
        sim->execNextEvent();
    }
    delete sim;
    delete world;
}
