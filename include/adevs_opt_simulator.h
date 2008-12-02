#ifndef __adevs_opt_simulator_h_
#define __adevs_opt_simulator_h_
#include "adevs.h"
#include "adevs_abstract_simulator.h"
#include "adevs_lp.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <omp.h>
#include "adevs_sched.h"

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
template <class X> class OptSimulator:
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
		OptSimulator(Devs<X>* model, int batch_size = 20);
		/// Get the model's next event time
		double nextEventTime()
		{
			Time tN = totalNextEventTime();
			return tN.t;
		}
		/// Get the complete next event time
		Time totalNextEventTime();
		/**
		 * Execute the simulation until the next event time is greater
		 * than the specified value.
		 */
		void execUntil(double gvt)
		{
			Time t_stop(gvt,UINT_MAX-1);
			if (gvt == DBL_MAX) t_stop = Time::Inf();
			execUntil(t_stop);
		}
		/**
		 * Execute the simulator until the next event time is greater
		 * than the specified value.
		 */
		void execUntil(Time gvt);
		/**
		Deletes the simulator, but leaves the model intact. The model must
		exist when the simulator is deleted.  Delete the model only after
		the simulator is deleted.
		*/
		~OptSimulator();
	private:
		// Logical processes that are run in parallel
		LogicalProcess<X>* lp;
		/// Number of lps
		const int lp_count;
		/// Number of events to execute in each iteration
		const int batch_size;
		/**
		 * Recursively initialize the model by assigning an lp to each atomic
		 * model and putting active models into the schedule.
		*/
		void initialize(Devs<X>* model, int& assign_to);
};

template <class X>
OptSimulator<X>::OptSimulator(Devs<X>* model, int batch_size):
	AbstractSimulator<X>(),
	lp_count(omp_get_max_threads()),
	batch_size(batch_size)
{
	lp = new LogicalProcess<X>[lp_count];
	int assign_to = 0;
	initialize(model,assign_to);
}

template <class X>
void OptSimulator<X>::initialize(Devs<X>* model, int& assign_to)
{
	Atomic<X>* a = model->typeIsAtomic();
	if (a != NULL)
	{
		lp[assign_to].addModel(a);
		assign_to = (assign_to+1)%lp_count;
	}
	else
	{
		Set<Devs<X>*> components;
		model->typeIsNetwork()->getComponents(components);
		typename Set<Devs<X>*>::iterator iter = components.begin();
		for (; iter != components.end(); iter++)
		{
			initialize(*iter,assign_to);
		}
	}
}

template <class X>
void OptSimulator<X>::execUntil(Time stop_time)
{
	for (;;)
	{
		int i;
		// Execute in parallel for a little while
		#pragma omp parallel for default(shared) private(i)
		for (i = 0; i < lp_count; i++)
		{
			for (int k = 0; k < batch_size; k++)
			{
				lp[i].processInput();
				lp[i].execEvents();
			}
		}
		// Flush the message buffers
		int cleared = 0;
		while (cleared != lp_count)
		{
			cleared = 0;
			for (i = 0; i < lp_count; i++)
				lp[i].processInput();
			for (i = 0; i < lp_count; i++)
			{
				if (!lp[i].pendingInput())
					cleared++;
			}
		}
		// Commit events and do fossil collection
		Time actual_gvt = totalNextEventTime();
		if (stop_time < Time::Inf() && stop_time < actual_gvt)
			actual_gvt = stop_time+Time(0.0,1);
		#pragma omp parallel for default(shared) private(i)
		for (i = 0; i < lp_count; i++)
		{
			lp[i].fossilCollect(actual_gvt,this);
		} 
		if (actual_gvt == Time::Inf() || stop_time < actual_gvt) break;
	}	
	// Do fossil collection and send event notifications
}

template <class X>
Time OptSimulator<X>::totalNextEventTime()
{
	Time gvt = Time::Inf();
	for (int i = 0; i < lp_count; i++)
		if (lp[i].getNextEventTime() <= gvt)
			gvt = lp[i].getNextEventTime();
	return gvt;
}

template <class X>
OptSimulator<X>::~OptSimulator()
{
	delete [] lp;
}

} // End of namespace

#endif
