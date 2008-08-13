#include "Clerk2.h"
#include "Generator.h"
#include "Observer.h"
#include <iostream>
using namespace std;
using namespace adevs;

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		cout << "Need an input file and output file!" << endl;
		return 1;
	}
	// Create the model
	Digraph<Customer*> store;
	Clerk2* clrk = new Clerk2();
	Generator* genr = new Generator(argv[1]);
	Observer* obsrv = new Observer(argv[2]);
	store.add(clrk);
	store.add(genr);
	store.add(obsrv);
	store.couple(genr,genr->arrive,clrk,clrk->arrive);
	store.couple(clrk,clrk->depart,obsrv,obsrv->departed);
	// Run the simulation
	Simulator<PortValue<Customer*> > sim(&store);
	while (sim.nextEventTime() < DBL_MAX)
	{
		sim.execNextEvent();
	}
	return 0;
}
