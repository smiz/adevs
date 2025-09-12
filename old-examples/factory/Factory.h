#ifndef _Factory_h_
#define _Factory_h_
#include <list>
#include <memory>
#include <set>
#include "Machine.h"
#include "adevs/adevs.h"


// This class implements the Factory and it machine usage policy.
class Factory : public adevs::Network<int> {
  public:
    Factory();

    void getComponents(set<adevs::Devs<int>*> &c);

    void route(int const &order, adevs::Devs<int>* src, std::list<adevs::Event<int>> &r);

    bool model_transition();

    int get_machine_count();

  private:
    // This is the machine set
    std::list<std::shared_ptr<Machine>> machines;

    // Method for adding a machine to the factory
    void add_machine();

    // Compute time needed for a machine to finish a new job
    double compute_service_time(std::shared_ptr<Machine> m);
};

#endif
