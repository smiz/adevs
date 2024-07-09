#include "Factory.h"
using namespace adevs;
using namespace std;

Factory::Factory()
    : Network<int>()  // call the parent constructor
{
    // Add the first machine the the machine set
    add_machine();
}

void Factory::getComponents(Set<Devs<int>*> &c) {
    // Copy the machine set to c
    list<Machine*>::iterator iter;
    for (iter = machines.begin(); iter != machines.end(); iter++) {
        c.insert(*iter);
    }
}

void Factory::route(int const &order, Devs<int>* src, Bag<Event<int>> &r) {
    // If this is a machine output, then it leaves the factory
    if (src != this) {
        r.push_back(Event<int>(this, order));
        return;
    }
    // Otherwise, look for the machine that has the shortest time to fill the order
    Machine* pick = NULL;        // No machine
    double pick_time = DBL_MAX;  // Infinite time for service
    list<Machine*>::iterator iter;
    for (iter = machines.begin(); iter != machines.end(); iter++) {
        // If the machine is available
        if ((*iter)->getQueueSize() <= 1) {
            double candidate_time = compute_service_time(*iter);
            // If the candidate machine service time is smaller than the current pick service time
            if (candidate_time < pick_time) {
                pick_time = candidate_time;
                pick = *iter;
            }
        }
    }
    // Make sure we found a machine to use and that it has a small enough service time
    assert(pick != NULL && pick_time <= 6.0);
    // Use this machine to process the order
    r.push_back(Event<int>(pick, order));
}

bool Factory::model_transition() {
    // Remove idle machines
    list<Machine*>::iterator iter = machines.begin();
    while (iter != machines.end()) {
        if ((*iter)->getQueueSize() == 0) {
            iter = machines.erase(iter);
        } else {
            iter++;
        }
    }
    // Add the new machine if we need it
    int spare_cap = 0;
    for (iter = machines.begin(); iter != machines.end(); iter++) {
        spare_cap += 2 - (*iter)->getQueueSize();
    }
    if (spare_cap == 0) {
        add_machine();
    }
    return false;
}

void Factory::add_machine() {
    machines.push_back(new Machine());
    machines.back()->setParent(this);
}

double Factory::compute_service_time(Machine* m) {
    // If the machine is working 3 days + queued orders * 3 + time for current order
    if (m->ta() < DBL_MAX) {
        return 3.0 + (m->getQueueSize() - 1) * 3.0 + m->ta();
    }
    // Otherwise needs 3 days
    else {
        return 3.0;
    }
}

int Factory::getMachineCount() {
    return machines.size();
}

Factory::~Factory() {
    // Delete all of the machines
    list<Machine*>::iterator iter;
    for (iter = machines.begin(); iter != machines.end(); iter++) {
        delete *iter;
    }
}
