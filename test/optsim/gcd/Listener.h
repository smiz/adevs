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
			omp_init_lock(&lock);
		}
		~Listener()
		{
			omp_destroy_lock(&lock);
		}
		void outputEvent(adevs::Event<PortValue> x, double t)
		{
		}
		void stateChange(adevs::Atomic<PortValue>* model, double t, void* state)
		{
			omp_set_lock(&lock);
			counter* c = dynamic_cast<counter*>(model);
			if (c != NULL) c->printState(state);
			genr* g = dynamic_cast<genr*>(model);
			if (g != NULL) g->printState(state);
			omp_unset_lock(&lock);
		}
	private:
		omp_lock_t lock;
};

#endif
