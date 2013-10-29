/**
 * Copyright (c) 2013, James Nutaro
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of the FreeBSD Project.
 *
 * Bugs, comments, and questions can be sent to nutaro@gmail.com
 */
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
 * be assigned randomly to a thread. Note that this simulator does not support dynamic
 * structure models.
 */
template <class X, class T = double> class ParSimulator:
   public AbstractSimulator<X,T>	
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
		ParSimulator(Devs<X,T>* model, MessageManager<X>* msg_manager = NULL);
		/**
		 * This constructor accepts a directed graph whose edges tell the
		 * simulator which processes feed input to which other processes.
		 * For example, a simulator with processors 1, 2, and 3 where 1 -> 2
		 * and 2 -> 3 would have two edges: 1->2 and 2->3.
		 */
		ParSimulator(Devs<X,T>* model, LpGraph& g,
			MessageManager<X>* msg_manager = NULL);
		/// Get the model's next event time
		T nextEventTime();
		/**
		 * Execute the simulator until the next event time is greater
		 * than the specified value. There is no global clock, 
		 * so this must be the actual time that you want to stop.
		 */
		void execUntil(T stop_time);
		/**
		 * Deletes the simulator, but leaves the model intact. The model must
		 * exist when the simulator is deleted, so delete the model only after
		 * the simulator is deleted.
		 */
		~ParSimulator();
	private:
		LogicalProcess<X,T>** lp;
		int lp_count;
		MessageManager<X>* msg_manager;
		void init(Devs<X,T>* model);
		void init_sim(Devs<X,T>* model, LpGraph& g);
}; 

template <class X, class T>
ParSimulator<X,T>::ParSimulator(Devs<X,T>* model, MessageManager<X>* msg_manager):
	AbstractSimulator<X,T>(),msg_manager(msg_manager)
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

template <class X, class T>
ParSimulator<X,T>::ParSimulator(Devs<X,T>* model, LpGraph& g,
		MessageManager<X>* msg_manager):
	AbstractSimulator<X,T>(),msg_manager(msg_manager)
{
	init_sim(model,g);
}

template <class X, class T>
void ParSimulator<X,T>::init_sim(Devs<X,T>* model, LpGraph& g)
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
	lp = new LogicalProcess<X,T>*[lp_count];
	for (int i = 0; i < lp_count; i++)
	{
		lp[i] = new LogicalProcess<X,T>(i,g.getI(i),g.getE(i),
			lp,this,msg_manager);
	}
	init(model);
}

template <class X, class T>
T ParSimulator<X,T>::nextEventTime()
{
	Time<T> tN = Time<T>::Inf();
	for (int i = 0; i < lp_count; i++)
	{
		if (lp[i]->getNextEventTime() < tN)
			tN = lp[i]->getNextEventTime();
	}
	return tN.t;
}

template <class X, class T>
ParSimulator<X,T>::~ParSimulator()
{
	for (int i = 0; i < lp_count; i++)
		delete lp[i];
	delete [] lp;
   delete msg_manager;	
}

template <class X, class T>
void ParSimulator<X,T>::execUntil(T tstop)
{
	#pragma omp parallel
	{
		lp[omp_get_thread_num()]->run(tstop);
	}
}

template <class X, class T>
void ParSimulator<X,T>::init(Devs<X,T>* model)
{
	if (model->getProc() >= 0 && model->getProc() < lp_count)
	{
		lp[model->getProc()]->addModel(model);
		return;
	}
	Atomic<X,T>* a = model->typeIsAtomic();
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
		Set<Devs<X,T>*> components;
		model->typeIsNetwork()->getComponents(components);
		typename Set<Devs<X,T>*>::iterator iter = components.begin();
		for (; iter != components.end(); iter++)
		{
			init(*iter);
		}
	}
}

} // end of namespace

#endif
