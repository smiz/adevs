#ifndef __adevs_par_simulator_h_
#define __adevs_par_simulator_h_
#include "adevs.h"
#include "adevs_abstract_simulator.h"
#include "adevs_lp.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>
#include "adevs_sched.h"
#include "adevs_lp_graph.h"

namespace adevs
{

/**
 * This class implements an optimistic simulation algorithm that uses
 * the OpenMP standard for its threading functionality. Your model
 * must satisfy four properties for this simulator to work properly:
 * (1) Every Atomic model must implement the methods for saving and restoring
 * its state, (2) Atomic models can not share any state variables (read
 * or write), (3) the route methods of all of the Network models must
 * be re-entrant, and (4) there are no structure changes. 
*/
template <class X> class ParSimulator:
   public AbstractSimulator<X>	
{
	public:
		/**
		Create a simulator for the provided model. The simulator
		constructor will fail and throw an adevs::exception if the
		time advance of any component atomic model is less than zero.
		The batch size parameter controls the potential degree of parallelism
		and parallel overhead; it is the number of models that will process
		an event in every iteration of the optimistic simulator.  
		*/
		ParSimulator(Devs<X>* model);
		ParSimulator(Devs<X>* model, LpGraph& g);
		/// Get the model's next event time
		double nextEventTime();
		/**
		 * Execute the simulator until the next event time is greater
		 * than the specified value.
		 */
		void execUntil(double stop_time);
		/**
		Deletes the simulator, but leaves the model intact. The model must
		exist when the simulator is deleted.  Delete the model only after
		the simulator is deleted.
		*/
		~ParSimulator();
	private:
		LogicalProcess<X>** lp;
		int lp_count;
		void init(Devs<X>* model);
		void init_sim(Devs<X>* model, LpGraph& g);
}; 

template <class X>
ParSimulator<X>::ParSimulator(Devs<X>* model):
	AbstractSimulator<X>()
{
	// Create an all to all coupling
	lp_count = omp_get_max_threads();
	LpGraph g;
	for (int i = 0; i < lp_count; i++)
	{
		for (int j = 0; j < lp_count; j++)
		{
			if (i != j)
			{
				g.addEdge(i,j);
				g.addEdge(j,i);
			}
		}
	}
	init_sim(model,g);
}

template <class X>
ParSimulator<X>::ParSimulator(Devs<X>* model, LpGraph& g):
	AbstractSimulator<X>()
{
	init_sim(model,g);
}

template <class X>
void ParSimulator<X>::init_sim(Devs<X>* model, LpGraph& g)
{
	lp_count = omp_get_max_threads();
	lp = new LogicalProcess<X>*[lp_count];
	for (int i = 0; i < lp_count; i++)
	{
		lp[i] = new LogicalProcess<X>(i,g.getI(i),g.getE(i),lp,this);
	}
	init(model);
}

template <class X>
double ParSimulator<X>::nextEventTime()
{
	Time tN = Time::Inf();
	for (int i = 0; i < lp_count; i++)
	{
		if (lp[i]->getNextEventTime() < tN)
			tN = lp[i]->getNextEventTime();
	}
	return tN.t;
}

template <class X>
ParSimulator<X>::~ParSimulator<X>()
{
	for (int i = 0; i < lp_count; i++)
		delete lp[i];
	delete [] lp; 
}

template <class X>
void ParSimulator<X>::execUntil(double tstop)
{
	#pragma omp parallel
	{
		lp[omp_get_thread_num()]->run(tstop);
	}
}

template <class X>
void ParSimulator<X>::init(Devs<X>* model)
{
	Atomic<X>* a = model->typeIsAtomic();
	if (a != NULL)
	{
		int lp_assign = a->getLP();
		if (lp_assign < 0 || lp_assign >= lp_count)
			lp_assign = ((long int)a)%lp_count;
		lp[lp_assign]->addModel(a);
	}
	else
	{
		Set<Devs<X>*> components;
		model->typeIsNetwork()->getComponents(components);
		typename Set<Devs<X>*>::iterator iter = components.begin();
		for (; iter != components.end(); iter++)
		{
			init(*iter);
		}
	}
}

} // End of namespace

#endif
