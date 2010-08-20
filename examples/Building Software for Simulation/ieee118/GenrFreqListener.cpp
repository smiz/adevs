#include "GenrFreqListener.h"
using namespace std;
using namespace adevs;

GenrFreqListener::GenrFreqListener(ElectricalModel* model, double cint, string model_name):
	EventListener<PortValue<BasicEvent*> >(),
	cint(cint),
	fout(string(model_name+"_freq.dat").c_str()),
	t_last(0.0),
	src(model)
{
}

void GenrFreqListener::stateChange(Atomic<PortValue<BasicEvent*> >* model, double t)
{
	if (t-t_last >= cint)
	{
		t_last = t;
		fout << t << " ";
		unsigned genrs = src->getElectricalData()->getGenrCount();
		for (unsigned i = 0; i < genrs; i++) 
			fout << src->getGenrFreq(i)/6.28 << " "; // In the pu system
		fout << endl;
	}
}

GenrFreqListener::~GenrFreqListener()
{
	fout.close();
}

