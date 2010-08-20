#include "GenrFail.h"
#include "ElectricalModel.h"
#include "IEEE118.h"
#include "LoadControl.h"
#include "QueueBus.h"
#include "GenrFreqListener.h"
#include "MechPowerListener.h"
#include "BusVoltsListener.h"
#include <map>
#include <set>
#include <fstream>
#include <cerrno>
using namespace std;
using namespace adevs;

class ControlListener:
	public EventListener<PortValue<BasicEvent*> >
{
	public:
		ControlListener(ElectricalModel* emodel, LoadControl* load_control):
			EventListener<PortValue<BasicEvent*> >(),
			emodel(emodel),load_control(load_control), 
			lout("control.dat"),
			vout("voltage.dat")
		{
			max_q_size = 0;
			cost[0] = cost[1] = 0.0;
			effectiveness[0] = effectiveness[1] = 0.0;
			tc = te = 0.0;
		}
		size_t getMaxQSize() const { return max_q_size; }
		double getCost() const { return cost[1]; }
		double getEffectiveness() const { return effectiveness[1]; }
		int getNumOffline() const { return offline.size(); }
		void outputEvent(Event<PortValue<BasicEvent*> > y, double t)
		{
			if (y.model == emodel)
			{
				GenrSampleEvent* s = dynamic_cast<GenrSampleEvent*>(y.value.value);
				if (s == NULL ) return;
				else if (s->freqBreakerOpen())
				{
					if (offline.find(s->getBusID()) == offline.end()) cerr << "Lost " << s->getBusID() << " @ t = " << t << endl;
					offline.insert(s->getBusID());
				}
			}
			else if (y.model == load_control && y.value.port == load_control->load_change)
			{
				LoadAdjustEvent* e = dynamic_cast<LoadAdjustEvent*>(y.value.value);
				assert(e != NULL);
				lout << t << " " << e->getAdjustment() << endl;
				cost[1] += (t-tc)*fabs(cost[0]);
				cost[0] = e->getAdjustment();
				tc = t;
			}
		}
		void stateChange(Atomic<PortValue<BasicEvent*> >* atomic , double t)
		{
			QueueBus* bus = dynamic_cast<QueueBus*>(atomic);
			if (bus != NULL)
			{
				max_q_size = max(bus->getPacketCount(),max_q_size);
			}
			else if (atomic == emodel)
			{
				unsigned genrs = emodel->getElectricalData()->getGenrCount();
				double f2sum = 0.0; unsigned ok = 0;
				for (unsigned i = 0; i < genrs; i++)
				{
					if (!emodel->genrOffLine(i))
					{
						ok++;
						double ftmp = 60.0*emodel->getGenrFreq(i);
						f2sum += (ftmp*ftmp);
					}
				}
				effectiveness[1] += (t-te)*effectiveness[0];
				effectiveness[0] = f2sum/(double)ok;
				te = t;
			}
		}
		~ControlListener() { lout.close(); vout.close(); }
	private:
		size_t max_q_size;
		double cost[2], effectiveness[2];
		double te, tc;
		ElectricalModel* emodel;
		LoadControl* load_control;
		set<unsigned> offline;
		ofstream lout, vout;
};

int main(int argc, const char** argv)
{
	// Get command line arguments
	if (argc <= 1)
	{
		cerr << "Need a packet rate, gain, freq levels, and genr #" << endl; 
		return -1;
	}
	double packet_rate = atof(argv[1]);
	double gain = atof(argv[2]);
	int freq_levels = atoi(argv[3]);
	int genr_id = atoi(argv[4]);
	cout << "params: " << packet_rate << " " << gain << " " << freq_levels << " " << genr_id << endl;
	// Create the model
	IEEE118* data = new IEEE118();
	LoadControl* control = new LoadControl(data,freq_levels,gain);
	GenrFail* genr_fail = new GenrFail(genr_id,1.0,data);
	ElectricalModel* dynamics = new ElectricalModel(new ElectricalModelEqns(data));
	Digraph<BasicEvent*>* model = new Digraph<BasicEvent*>();
	model->add(control);
	model->add(genr_fail);
	model->add(dynamics);
	model->couple(genr_fail,genr_fail->genr_fail,dynamics,ElectricalModelEqns::GenrTrip);
	model->couple(control,control->sample_setting,dynamics,ElectricalModelEqns::SetGenrSample);
	for (unsigned i = 0; i < data->getNodeCount(); i++)
	{
		model->couple(dynamics,i,control,control->sample_arrive);
		QueueBus* aggr_net = new QueueBus(packet_rate);
		model->add(aggr_net);
		model->couple(control,control->load_change,aggr_net,aggr_net->load);
		model->couple(aggr_net,aggr_net->load,dynamics,ElectricalModelEqns::LoadAdj);
	}
	// Simulate it
	ControlListener* control_listener = new ControlListener(dynamics,control);
	GenrFreqListener* freq_listener = new GenrFreqListener(dynamics,1E-4);
	MechPowerListener* pm_listener = new MechPowerListener(dynamics,1E-4);
	BusVoltsListener* volt_listener = new BusVoltsListener(dynamics,1E-4);
	Simulator<PortValue<BasicEvent*> >* sim =
		new Simulator<PortValue<BasicEvent*> >(model);
	sim->addEventListener(control_listener);
	sim->addEventListener(freq_listener);
	sim->addEventListener(volt_listener);
	sim->addEventListener(pm_listener);
	int count = 0;
	// Dump the initial voltages
	ofstream vout("init_voltage.dat");
	for (unsigned i = 0; i < dynamics->getElectricalData()->getNodeCount(); i++)
		vout << abs(dynamics->getVoltage(i)) << endl;
	vout.close(); 
	while (sim->nextEventTime() <= 10.0)
	{
		if ((count++)%10000 == 0) cerr << sim->nextEventTime() << endl;
		sim->execNextEvent();
	} 
	// Dump the final voltages
	vout.open("final_voltage.dat");
	for (unsigned i = 0; i < dynamics->getElectricalData()->getNodeCount(); i++)
		vout << abs(dynamics->getVoltage(i)) << endl;
	vout.close();
	delete model;
	delete sim;
	cout << "results: " << control_listener->getMaxQSize() << " "
		<< control_listener->getCost() << " " << control_listener->getEffectiveness() <<
		" " << control_listener->getNumOffline() << endl;
	delete control_listener;
	delete freq_listener;
	delete volt_listener;
	delete pm_listener;
	return 0;
}

