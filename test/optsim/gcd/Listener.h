#ifndef _Listener_h_
#define _Listener_h_
#include <iostream>
#include "adevs.h"
#include "gcd.h"

class Listener:
	public adevs::EventListener<PortValue>
{
	public:
		Listener()
		{
		}
		~Listener()
		{
		}
		void outputEvent(adevs::Event<PortValue> x, double t)
		{
		}
		void stateChange(adevs::Atomic<PortValue>* model, double t, void* state)
		{
			counter* c = dynamic_cast<counter*>(model);
			if (c != NULL) c->printState(state);
			genr* g = dynamic_cast<genr*>(model);
			if (g != NULL) g->printState(state);
		}
};

#endif
