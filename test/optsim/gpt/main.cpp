#include <cstdlib>
#include <iostream>
#include "adevs.h"
#include "job.h"
#include "proc.h"
#include "genr.h"
#include "transd.h"
using namespace std;

genr* gnr = NULL;
transd* trnsd = NULL;
proc* prc = NULL;

class Listener:
	public adevs::EventListener<adevs::PortValue<job> >
{
	public:
		Listener():
			adevs::EventListener<adevs::PortValue<job> >()
		{
		}
		~Listener()
		{
		}
		void outputEvent(adevs::Event<adevs::PortValue<job> > x, double t)
		{
			if (x.model == gnr && x.value.port == gnr->out)
				cout << t << " genr!out" << endl;
			else if (x.model == trnsd && x.value.port == trnsd->out)
				cout << t << " transd!out" << endl;
			else if (x.model == prc && x.value.port == prc->out)
				cout << t << " proc!out" << endl;
			else
				assert(false);
		}
		void stateChange(adevs::Atomic<adevs::PortValue<job> >* model, double t, void* state)
		{
			cout << "State change @ t = " << t;
			if (model == prc) cout << " : proc" << endl;
			else if (model == gnr) cout << " : genr" << endl;
			else if (model == trnsd)
			{
				cout << " : transd" << endl;
				trnsd->printSummary(state);
			}
			else
				assert(false);
		}
	private:
};

int main() 
{
	/// Get experiment parameters
	double g, p, t;
	cout << "Genr period: ";
	cin >> g;
	cout << "Proc time: ";
	cin >> p;
	cout << "Observation time: ";
	cin >> t;
	cout << endl;
	/// Create and connect the atomic components using a digraph model.
	adevs::Digraph<job> model;
	gnr = new genr(g);
	trnsd = new transd(t);
	prc = new proc(p);
	/// Add the components to the digraph
	model.add(gnr);
	model.add(trnsd);
	model.add(prc);
	/// Establish component coupling
	model.couple(gnr, gnr->out, trnsd, trnsd->ariv);
	model.couple(gnr, gnr->out, prc, prc->in);
	model.couple(prc, prc->out, trnsd, trnsd->solved);
	model.couple(trnsd, trnsd->out, gnr, gnr->stop);
	/// Create a simulator for the model and run it until
	/// the model passivates.
	adevs::OptSimulator<PortValue> sim(&model);
	sim.addEventListener(new Listener());
	sim.execUntil(DBL_MAX);
	/// Done!
	return 0;
}
