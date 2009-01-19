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
		void stateChange(adevs::Atomic<PortValue>* model, double t)
		{
			counter* c = dynamic_cast<counter*>(model);
			if (c != NULL)
			{
				#pragma omp critical
				{
					c->printState();
				}
			}
			genr* g = dynamic_cast<genr*>(model);
			if (g != NULL)
			{
				#pragma omp critical
				{
					g->printState();
				}
			}
		}
};

#endif
