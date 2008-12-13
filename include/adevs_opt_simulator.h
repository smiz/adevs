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
		OptSimulator(Devs<X>* model);
		/// Get the model's next event time
		double nextEventTime() { return gvt; }
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
		~OptSimulator();
		/// Get the number of logical processes
		int getNumLPs() const { return lp_count; }
		/// Get the performance data for an LP
		LP_perf_t getPerfData(int i) const
		{
			return lp[i].getPerfData();
		} 
	private:
		// Logical processes that are run in parallel
		LogicalProcess<X>* lp;
		/// Number of lps
		const int lp_count;
		/// For gvt calculations
		double gvt, *gvt_array;
		int run_gvt;
		omp_lock_t gvt_lock;
		/**
		 * Recursively initialize the model by assigning an lp to each atomic
		 * model and putting active models into the schedule.
		*/
		void initialize(Devs<X>* model, int& assign_to);

		void calc_gvt(int i);
		void contrib_gvt(int i);

};

template <class X>
OptSimulator<X>::OptSimulator(Devs<X>* model):
	AbstractSimulator<X>(),
	lp_count(omp_get_max_threads())
{
	omp_init_lock(&gvt_lock);
	gvt = 0.0;
	run_gvt = 0;
	gvt_array = new double[lp_count];
	lp = new LogicalProcess<X>[lp_count];
	int assign_to = 0;
	initialize(model,assign_to);
	for (int i = 0; i < lp_count; i++)
	{
		gvt_array[i] = -1.0;
	}
}

template <class X>
void OptSimulator<X>::initialize(Devs<X>* model, int& assign_to)
{
	Atomic<X>* a = model->typeIsAtomic();
	if (a != NULL)
	{
		char pick = a->getPrefLP();
		if (pick < lp_count && pick >= 0) assign_to = pick;
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
void OptSimulator<X>::execUntil(double stop_time)
{
	int i;
	#pragma omp parallel for default(shared) private(i)
	for (i = 0; i < lp_count; i++)
	{
		for (;;)
		{
			#pragma omp flush(gvt)
			if (lp[i].getGarbageCount() > 100 || !lp[i].hasEvents())
			{
				calc_gvt(i);
				if (gvt <= stop_time)
					lp[i].fossilCollect(Time(gvt,0),this);
			}
			if (gvt < DBL_MAX && gvt <= stop_time)
			{
				contrib_gvt(i);
				lp[i].processInput();
				lp[i].execEvents();
			}
			else break;
		}
	}
	#pragma omp parallel for default(shared) private(i)
	for (i = 0; i < lp_count; i++)
	{
		lp[i].fossilCollect(Time(stop_time,UINT_MAX),this);
	}
}

template <class X>
void OptSimulator<X>::contrib_gvt(int i)
{
	omp_set_lock(&gvt_lock);
	if (run_gvt > 0)
	{
		double lvt = lp[i].getLVT();
		if (gvt_array[i] < 0.0)
		{
			gvt_array[i] = lvt;
			run_gvt--;
		}
		else if (lvt < gvt_array[i])
		{
			gvt_array[i] = lvt;
		}
		if (lp[i].getSendMin() < gvt_array[i])
			gvt_array[i] = lp[i].getSendMin();
		if (run_gvt == 0)
		{
			double gvt_tmp = DBL_MAX;
			for (int j = 0; j < lp_count; j++)
			{
				gvt_tmp = std::min(gvt_tmp,gvt_array[j]);
				gvt_array[j] = -1.0;
			}
			assert(gvt_tmp >= gvt);
			gvt = gvt_tmp;
		}
	}
	if (run_gvt == 0) lp[i].clearSendMin();
	omp_unset_lock(&gvt_lock);
}

template <class X>
void OptSimulator<X>::calc_gvt(int i)
{
	omp_set_lock(&gvt_lock);
	if (run_gvt == 0)
	{
		run_gvt = lp_count;
		lp[i].clearSendMin();
	}
	omp_unset_lock(&gvt_lock);
}

template <class X>
OptSimulator<X>::~OptSimulator()
{
	omp_destroy_lock(&gvt_lock);
	delete [] gvt_array;
	delete [] lp;
}

} // End of namespace

#endif
