#include <iostream>
#include "node.h"
#include "Listener.h"
using namespace std;

int main () 
{
	adevs::Digraph<token_t*>* model = new adevs::Digraph<token_t*>();
	node* n1 = new node(0,1,new token_t(0),true);
	node* n2 = new node(1,1,NULL);
	node* n3 = new node(2,1,new token_t(1),true);
	node* n4 = new node(3,1,NULL);
	model->add(n1);
	model->add(n2);
	model->add(n3);
	model->add(n4);
	model->couple(n1,n1->out,n2,n2->in);
	model->couple(n2,n2->out,n3,n3->in);  
	model->couple(n3,n3->out,n4,n4->in);  
	model->couple(n4,n4->out,n1,n1->in);  
	adevs::ParSimulator<PortValue>* sim = new adevs::ParSimulator<PortValue>(model);
	sim->addEventListener(new Listener());
	sim->execUntil(15.5);
	cout << "End of run!" << endl;
	delete sim;
	delete model;
	return 0;
}
