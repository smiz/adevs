#include "Clerk.h"
#include "Generator.h"
#include "Observer.h"
#include <iostream>
using namespace std;

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		cout << "Need input and output files!" << endl;
		return 1;
	}
	// Create a digraph model whose components use PortValue<Customer*>
	// objects as input and output objects.
	adevs::Digraph<Customer*> store;
	// Create and add the component models
	Clerk* clrk = new Clerk();
	Generator* genr = new Generator(argv[1]);
	Observer* obsrv = new Observer(argv[2]);
	store.add(clrk);
	store.add(genr);
	store.add(obsrv);
	// Couple the components
	store.couple(genr,genr->arrive,clrk,clrk->arrive);
	store.couple(clrk,clrk->depart,obsrv,obsrv->departed);
	// Create a simulator and run until its done
	adevs::Simulator<IO_Type> sim(&store);
	while (sim.nextEventTime() < DBL_MAX)
	{
		sim.execNextEvent();
	}
	// Done, component models are deleted when the Digraph is
	// deleted.
	return 0;
}
