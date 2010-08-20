#include "MechPowerListener.h"
using namespace std;
using namespace adevs;

MechPowerListener::MechPowerListener(ElectricalModel* model, double cint, string model_name):
	EventListener<PortValue<BasicEvent*> >(),
	cint(cint),
	fout(string(model_name+"_pm.dat").c_str()),
	t_last(0.0),
	src(model)
{
}

void MechPowerListener::stateChange(Atomic<PortValue<BasicEvent*> >* model, double t)
{
	if (t-t_last >= cint)
	{
		t_last = t;
		fout << t << " ";
		unsigned genrs = src->getElectricalData()->getGenrCount();
		for (unsigned i = 0; i < genrs; i++) 
			fout << src->getMechPower(i) << " ";
		fout << endl;
	}
}

MechPowerListener::~MechPowerListener()
{
	fout.close();
}

