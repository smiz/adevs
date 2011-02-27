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
#include "adevs_abstract_simulator.h"
#include "adevs_msg_manager.h"
#include "adevs_lp.h"
#include "adevs_lp_graph.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <cstdio>

namespace adevs
{

/**
 * This is the conservative simulator described in "Building Software for Simulation".
 * Models, network and atomic, can be assigned to specific threads (processors) by calling the
 * setProc() method. The components of a network will inherit its thread assignment.
 * Model's with an explicit assignment must have a positive lookahead. Atomic models that are
 * unassigned, by inheritance or otherwise, must have a positive lookahead and will
 * be assigned randomly to a thread.
 */
template <class X> class ParSimulator:
   public AbstractSimulator<X>	
{
	public:
		/**
		 * Create a simulator for the provided model. The Atomic components will
		 * be assigned to the preferred processors, or assigned randomly if no
		 * preference is given or the preference can not be satisfied. The
		 * message manager is used to handle inter-thread events. If msg_manager
		 * is NULL, the assignment and copy constructors of output objects 
		 * are used and their is no explicit cleanup (see the MessageManager
		 * documentation). This constructor assumes all to all connection of the
		 * processors.
		 */
		ParSimulator(Devs<X>* model, MessageManager<X>* msg_manager = NULL);
		/**
		 * This constructor accepts a directed graph whose edges tell the
		 * simulator which processes feed input to which other processes.
		 * For example, a simulator with processors 1, 2, and 3 where 1 -> 2
		 * and 2 -> 3 would have two edges: 1->2 and 2->3.
		 */
		ParSimulator(Devs<X>* model, LpGraph& g,
			MessageManager<X>* msg_manager = NULL);
		/// Get the model's next event time
		double nextEventTime();
		/**
		 * Execute the simulator until the next event time is greater
		 * than the specified value. There is no global clock, 
		 * so this must be the actual time that you want to stop.
		 */
		void execUntil(double stop_time);
		/**
		 * Deletes the simulator, but leaves the model intact. The model must
		 * exist when the simulator is deleted, so delete the model only after
		 * the simulator is deleted.
		 */
		~ParSimulator();
	private:
		LogicalProcess<X>** lp;
		int lp_count;
		MessageManager<X>* msg_manager;
		void init(Devs<X>* model);
		void init_sim(Devs<X>* model, LpGraph& g);
}; 

template <class X>
ParSimulator<X>::ParSimulator(Devs<X>* model, MessageManager<X>* msg_manager):
	AbstractSimulator<X>(),msg_manager(msg_manager)
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
ParSimulator<X>::ParSimulator(Devs<X>* model, LpGraph& g,
		MessageManager<X>* msg_manager):
	AbstractSimulator<X>(),msg_manager(msg_manager)
{
	init_sim(model,g);
}

template <class X>
void ParSimulator<X>::init_sim(Devs<X>* model, LpGraph& g)
{
	if (msg_manager == NULL) msg_manager = new NullMessageManager<X>();
	lp_count = g.getLPCount();
	if (omp_get_max_threads() < lp_count)
	{
		char buffer[1000];
		sprintf(buffer,"More LPs than threads. Set OMP_NUM_THREADS=%d.",
			lp_count);
		exception err(buffer);
		throw err;
	}
	omp_set_num_threads(lp_count);
	lp = new LogicalProcess<X>*[lp_count];
	for (int i = 0; i < lp_count; i++)
	{
		lp[i] = new LogicalProcess<X>(i,g.getI(i),g.getE(i),
			lp,this,msg_manager);
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
   delete msg_manager;	
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
	if (model->getProc() >= 0 && model->getProc() < lp_count)
	{
		lp[model->getProc()]->addModel(model);
		return;
	}
	Atomic<X>* a = model->typeIsAtomic();
	if (a != NULL)
	{
		int lp_assign = a->getProc();
		if (lp_assign < 0 || lp_assign >= lp_count)
			lp_assign =
				((unsigned long int)(a)^(unsigned long int)(this))%lp_count;
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

} // end of namespace

#endif
