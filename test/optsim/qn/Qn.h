#ifndef _Qn_h_
#define _Qn_h_
#include "adevs.h"
#include "Genr.h"
#include "Sq.h"
#include <iostream>

class Qn: public adevs::Network<int>
{
	public:
		Qn(int q, int s, adevs::LpGraph& lpg, bool recycle = false):
			adevs::Network<int>(),qcount(s),ql(new Ql*[s]),
			genr((double)s/2.0),collect(),recycle(recycle)
		{
			// Get the number of threads
			thrds = omp_get_max_threads();
			// The generators go in one LP
			genr.setParent(this);
			genr.setProc(0);
			// Collector goes on the last LP
			collect.setParent(this);
			collect.setProc(thrds-1);
			// The queues are split among the remainder
			for (int i = 0; i < qcount; i++) {
				ql[i] = new Ql(q);
				ql[i]->setParent(this);
				ql[i]->setProc(i%thrds);
			}
			// Add 0->[1,thrds) to the lp graph. This connects
			// the generator to everyone.
			for (int i = 1; i < thrds; i++) 
				lpg.addEdge(0,i);
			// Add [0,thrds-1),thrds-1] to the lp graph. This connects
			// everyone to the collector.
			for (int i = 0; i < thrds-1; i++)
			{
				lpg.addEdge(i,thrds-1);
			}
			// Recycled output goes to the first LP
			if (recycle)
			{
				lpg.addEdge(thrds-1,0);
			}
		}
		void getComponents(adevs::Set<adevs::Devs<int>*>& c)
		{
			for (int i = 0; i < qcount; i++) c.insert(ql[i]);
			c.insert(&genr); c.insert(&collect);
		}
		void route(const int &value, adevs::Devs<int> *model,
			adevs::Bag<adevs::Event<int> > &r) {
			if (model == &genr) r.insert(
					adevs::Event<int>(ql[value%qcount],value));
			else r.insert(adevs::Event<int>(&collect,value));
			if (recycle &&
					model != &genr &&
					model->getProc() == thrds-1 &&
					value % 2 == 0)
			{
				assert(ql[0]->getProc() == 0);
				r.insert(adevs::Event<int>(ql[0],value+1));
			}
		}
		~Qn()
		{
			for (int i = 0; i < qcount; i++)
				delete ql[i];
			delete [] ql;
		}
	private:
		const int qcount;
		Ql** ql;
		Genr genr;
		Collector collect;
		bool recycle;
		int thrds;
};

#endif
