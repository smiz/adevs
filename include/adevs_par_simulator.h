/***************
Copyright (C) 2009 by James Nutaro

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Bugs, comments, and questions can be sent to nutaro@gmail.com
***************/
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
 * This is a conservative simulator implemented using OpenMP. To work, your models must have
 * a positive lookahead and your event listeners must be thread safe. This simulator
 * is a little more restrictive than the single processor Simulator. You can not
 * inject input into a running simulation, and you must tell it when to stop. The simulator
 * will not halt automatically when there are no more events left (there is no global
 * simulation clock, and so time just keeps creeping forward until the specified
 * end time is reached). MAKE SURE THAT THE DEFAULT CONSTRUCTOR FOR YOUR IO TYPE
 * MAKES SENSE! It will be used extensively by the simulator.
*/
template <class X> class ParSimulator:
   public AbstractSimulator<X>	
{
	public:
		/**
		 * Create a simulator for the provided model. The Atomic components will
		 * be assigned to the prefered processors, or assigned randomly if no
		 * preference is given or the preference can not be satisfied.
		*/
		ParSimulator(Devs<X>* model);
		/**
		 * This constructor also accepts a directed graph whose edges tell the
		 * simulator which processes feed input to which other processes.
		 * For example, a simulator with processors 1, 2, and 3 where 1 -> 2
		 * and 2 -> 3 would have two edges: 1->2 and 2->3.
		 */
		ParSimulator(Devs<X>* model, LpGraph& g);
		/// Get the model's next event time
		double nextEventTime();
		/**
		 * Execute the simulator until the next event time is greater
		 * than the specified value. There is no global clock, and
		 * so this must be the actual time that you want to stop.
		 */
		void execUntil(double stop_time);
		/**
		 * Deletes the simulator, but leaves the model intact. The model must
		 * exist when the simulator is deleted.  Delete the model only after
		 * the simulator is deleted.
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
