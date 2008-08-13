#include "adevs.h"
#include "adevs_rk4.h"
#include "Mixer.h"
#include "Oven.h"
#include "Cleaner.h"
#include "LengthMonitor.h"
#include "OvenControl.h"
#include <iostream>
#include <cmath>
using namespace std;
using namespace adevs;

class CookieBaker: public Digraph<double>
{
	public:

		static const int in;
		static const int out;
		static const int alert;

		CookieBaker():
		Digraph<double>()
		{
			Oven* oven = new Oven();
			LengthMonitor* len_monitor = new LengthMonitor(2);
			Cleaner* cleaner = new Cleaner(5,1.0);
			OvenControl* control = new OvenControl();
			add(oven);
			add(len_monitor);
			add(cleaner);
			add(control);
			couple(this,this->in,cleaner,cleaner->in);
			couple(this,this->in,oven,oven->in);
			couple(this,this->in,len_monitor,len_monitor->in);
			couple(oven,oven->out,len_monitor,len_monitor->out);
			couple(oven,oven->out,cleaner,cleaner->out);
			couple(oven,oven->out,this,this->out);
			couple(len_monitor,len_monitor->status,control,control->q_status);
			couple(cleaner,cleaner->status,control,control->cleaner_status);
			couple(control,control->status,this,this->alert);
		}
};

const int CookieBaker::in = 0;
const int CookieBaker::out = 1;
const int CookieBaker::alert = 2;

// Component models
static Mixer* mixer = NULL;
static CookieBaker* baker = NULL;
// State variables to plot
static int dough_balls = 0;
static int cookies = 0;
static int queue_len = 0;
static double signal = 1.0;
static bool record = true;
static double dough = 0.0;

class Listener: public EventListener<PortValue<double> >
{
	public:
		Listener():
		EventListener<PortValue<double> >()
		{
		}
		void outputEvent(Event<PortValue<double> > x, double t)
		{
			record = true;
			if (x.model == mixer && x.value.port == mixer->dough_ball)
			{
				dough_balls++;
				queue_len++;
			}
			else if (x.model == mixer && x.value.port == mixer->dough)
			{
				dough = x.value.value;
			}
			else if (x.model == baker && x.value.port == baker->out)
			{
				queue_len--;
				cookies++;
			}
			else if (x.model == baker && x.value.port == baker->alert)
			{
				signal = x.value.value;
			}
		}
		~Listener(){}
};


int main()
{
	mixer = new Mixer();
	baker = new CookieBaker();
	Digraph<double>* model = new Digraph<double>();
	model->add(mixer);
	model->add(baker);
	model->couple(mixer,mixer->dough_ball,baker,baker->in);
	model->couple(baker,baker->alert,mixer,mixer->knob);
	Simulator<PortValue<double> >* sim = new Simulator<PortValue<double> >(model);
	Listener* l = new Listener();
	sim->addEventListener(l);
	double tL = 0.0;
	cout << "# Time Dough_balls Cookies Q_len Switch Dough" << endl;
	while (sim->nextEventTime() < 20.0)
	{
		if (record)
		{
			cout << tL << " " << dough_balls << " " << cookies << " " 
			<< queue_len << " " << signal << " " << dough << endl;
			record = false;
		}
		sim->execNextEvent();
		tL = sim->nextEventTime();
	}
	delete sim;
	delete model;
	delete l;
	return 0;
}
