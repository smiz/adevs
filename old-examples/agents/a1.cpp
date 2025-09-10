/**
 * This calculates an "agent based" solution to dx/dt = -ax.
 */
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <cmath>
#include <iostream>
#include <memory>
#include "adevs/adevs.h"

// using namespace adevs;
using Atomic = adevs::Atomic<int>;
using SimpleDigraph = adevs::SimpleDigraph<int>;


bool const verbose = false;
static int num_agents = 2000000;
static double const a = 1.0;
static gsl_rng* rnd = gsl_rng_alloc(gsl_rng_default);

class Agent : public Atomic<int> {
  public:
    Agent() : Atomic<int>(), ttg(gsl_ran_exponential(rnd, a)) { pop++; }
    void delta_int() { ttg = adevs_inf<double>(); }
    void delta_ext(double, std::list<int> const &) {}
    void delta_conf(std::list<int> const &) {}
    void output_func(std::list<int> &) { pop--; }
    double ta() { return ttg; }
    static int getPop() { return pop; }

  private:
    static int pop;
    double ttg;
};

int Agent::pop = 0;

double run() {

    double max_error = 0.0;

    std::shared_ptr<SimpleDigraph> world = std::make_shared<SimpleDigraph>();

    for (int i = 0; i < num_agents; i++) {
        world->add(std::make_shared<Agent>());
    }

    std::shared_ptr<Simulator> sim = std::make_shared<Simulator>(world);

    if (verbose) {
        std::cout << 0 << " " << ((double)(Agent::getPop()) / (double)(num_agents))
             << " " << exp(-a * 0.0) << std::endl;
    }

    while (sim->nextEventTime() < adevs_inf<double>()) {
        double t = sim->nextEventTime();
        sim->execNextEvent();
        double asoln = ((double)(Agent::getPop()) / (double)(num_agents));
        double tsoln = exp(-a * t);
        double err = asoln - tsoln;
        max_error = ::max(fabs(err), max_error);
        if (verbose) {
            std::cout << t << " " << asoln << " " << tsoln << " " << err << std::endl;
        }
    }
    return max_error;
}

int main() {
    //for (num_agents = 10000; num_agents < 5000000; num_agents += 10000) {
    for (num_agents = 10000; num_agents < 100000; num_agents += 10000) {
        double err = run();
        std::cout << num_agents << " " << err << std::endl;
    }
    return 0;
}
