#ifndef _AssemblyLine_h_
#define _AssemblyLine_h_
#include <set>
#include "Machine.h"

class AssemblyLine : public adevs::Network<int> {
  public:
    AssemblyLine()
        : adevs::Network<int>(),
          press(1.0),
          drill(2.0)  // Create the component models
    {
        // Remember to set their parent, otherwise the Simulator will
        // not be able to route their output events
        press.setParent(this);
        drill.setParent(this);
    }
    void getComponents(set<adevs::Devs<int>*> &c) {
        c.insert(&press);
        c.insert(&drill);
    }
    void route(int const &value, adevs::Devs<int>* model,
               list<adevs::Event<int>> &r) {
        adevs::Event<int> x;
        x.value = value;
        // External input to the network goes to the press
        if (model == this) {
            x.model = &press;
        }
        // Output from the drill leaves the assembly line
        else if (model == &drill) {
            x.model = this;
        }
        // Output from the press goes to the drill
        else if (model == &press) {
            x.model = &drill;
        }
        r.push_back(x);
    }
    Machine* getPress() { return &press; }
    Machine* getDrill() { return &drill; }

  private:
    Machine press, drill;
};

#endif
