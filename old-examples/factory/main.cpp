#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include "Factory.h"
#include "Genr.h"

// using namespace adevs;


/**
 * The observer will keep track of individual order service times.
 */
class Observer : public EventListener<int> {
  public:
    Observer() : EventListener<int>() {
        max_time = avg_time = 0.0;
        count = 0;
        min_time = DBL_MAX;  // this should be 3 at the end of a run
    }

    // Track order processing statistics as orders move through the system
    void outputEvent(Event<int> x, double t) {
        // Ignore machine outputs, just look at factory and generator events
        if (dynamic_cast<Machine*>(x.model) != NULL) {
            return;
        }
        // Put new orders into the order table
        if (orders.find(x.value) == orders.end()) {
            orders[x.value] = t;
        }
        // Compute statistics for orders that are complete
        else {
            // Maximum time to process any order
            max_time = std::max(max_time, t - orders[x.value]);
            // Min time to process any order
            min_time = std::min(min_time, t - orders[x.value]);
            // Update the average
            count++;
            avg_time += t - orders[x.value];
            // Clear order from the table
            orders.erase(x.value);
        }
    }
    // Get the maximum service time
    double maxServiceTime() { return max_time; }
    // Get the minimum service time
    double minServiceTime() { return min_time; }
    // Get the average service time
    double avgServiceTime() { return avg_time / count; }

  private:
    std::map<int, double> orders;
    double max_time, avg_time, min_time;
    int count;
};

int main(int argc, char** argv) {
    // Create the model
    std::shared_ptr<Factory> factory = std::make_shared<Factory>();

    long seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);  // Seed from command line argument
    }

    std::shared_ptr<Genr> genr = std::make_shared<Genr>(seed);

    std::shared_ptr<SimpleDigraph<int>> model = std::make_shared<SimpleDigraph<int>>();
    model->add(factory);
    model->add(genr);
    model->couple(genr, factory);

    // Create the simulator
    std::shared_ptr<Simulator<int>> sim = std::make_shared<Simulator<int>>(model);

    // Add an observer
    std::shared_ptr<Observer> obs = std::make_shared<Observer>();
    sim->addEventListener(obs);

    // Initial active count (should be 0)
    std::cout << "0 " << factory->get_machine_count() << estd::ndl;

    // Run the simulation and output active machine count at each iteration
    while (sim->nextEventTime() <= 365.0) {
        std::cout << sim->nextEventTime() << " ";
        sim->execNextEvent();
        std::cout << factory->get_machine_count() << std::endl;
    }

    // Output service time statistics
    std::cerr << "Avg. service time: " << obs->avgServiceTime() << " days" << std::endl;
    std::cerr << "Max. service time: " << obs->maxServiceTime() << " days" << std::endl;
    std::cerr << "Min. service time: " << obs->minServiceTime() << " days" << std::endl;

    return 0;
}
