#include <iostream>
#include "node.h"
#include "Listener.h"
using namespace std;

int main () 
{
	adevs::Digraph<token_t*>* model = new adevs::Digraph<token_t*>();
	node* n1 = new node(0,0,new token_t());
	node* n2 = new node(1,0,new token_t(1));
	model->add(n1);
	model->add(n2);
	model->couple(n1,n1->out,n2,n2->in);
	model->couple(n2,n2->out,n1,n1->in);  
	adevs::OptSimulator<PortValue>* sim = new adevs::OptSimulator<PortValue>(model);
	sim->addEventListener(new Listener());
	sim->execUntil(adevs::Time(0.0,11));
	cout << "End of run!" << endl;
	delete sim;
	delete model;
	return 0;
}
