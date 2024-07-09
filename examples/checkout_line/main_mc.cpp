#include <iostream>
#include "Clerk.h"
#include "Clerk2.h"
#include "Customer.h"
#include "Generator.h"
#include "MultiClerk.h"
#include "Observer.h"
#include "adevs.h"
using namespace std;
using namespace adevs;

int main(int argc, char** argv) {
    if (argc != 3) {
        cout << "Need an input and output file!" << endl;
        return 1;
    }
    Digraph<Customer*> store;
    MultiClerk* clrk = new MultiClerk();
    Generator* genr = new Generator(argv[1]);
    Observer* obsrv = new Observer(argv[2]);
    store.add(clrk);
    store.add(genr);
    store.add(obsrv);
    store.couple(genr, genr->arrive, clrk, clrk->arrive);
    store.couple(clrk, clrk->depart, obsrv, obsrv->departed);
    Simulator<IO_Type> sim(&store);
    while (sim.nextEventTime() < 100.0) {
        sim.execNextEvent();
    }
    return 0;
}
