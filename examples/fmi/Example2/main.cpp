#include "io.h"
#include "adevs.h"
#include "adevs_fmi.h"
#include "Ethernet.h"
#include "Control.h"
#include <cmath>
#include <iostream>
using namespace std;
using namespace adevs;

static const int controlAddr = 1;
static const int robotAddr = 2;
static int numApps = 0;

class App:
	public AtomicModel
{
	public:
		static const int data_out;

		App(double freq, int bytes):
			AtomicModel(),
			freq(freq),
			bytes(bytes)
		{
		}
		double ta() { return randVar.exponential(1.0/freq); }
		void delta_int(){}
		void delta_ext(double,const Bag<IO_Type>&){}
		void delta_conf(const Bag<IO_Type>&){}
		void gc_output(Bag<IO_Type>&){}
		void output_func(Bag<IO_Type>& yb)
		{
			IO_Type y;
			y.port = data_out;
			y.value =
				new NetworkData(NetworkData::APP_DATA,999,bytes,new SimObject());
			yb.insert(y);
		}
	private:
		const double freq;
		const int bytes;
		static adevs::rv randVar;
};

const int App::data_out = 0;
adevs::rv App::randVar(new crand());

class ControlExt:
	public Control
{
	public:
		static const int sample;
		static const int command;

		ControlExt():
			Control(),
			doCmd(false)
		{
			for (int i = 0; i < 2; i++)
				T[i] = err[i] = ierr[i] = 0.0;
		}
		double time_event_func(const double* q) {
			double tSup = Control::time_event_func(q);
			if (doCmd) return 0.0;
			else return tSup;
		}
		void internal_event(double* q, const bool* state_event) {
			Control::internal_event(q,state_event);
			doCmd = false;
		}
		void external_event(double* q, double e,
			const adevs::Bag<IO_Type>& xb) {
			Control::external_event(q,e,xb);
			process_input_data(e,xb);
			Control::external_event(q,e,xb);
		}
		void confluent_event(double *q, const bool* state_event,
			const adevs::Bag<IO_Type>& xb) {
			double h = time_event_func(q);
			Control::confluent_event(q,state_event,xb);
			process_input_data(h,xb);
			Control::confluent_event(q,state_event,xb);
		}
		void output_func(const double *q, const bool* state_event,
           adevs::Bag<IO_Type>& yb) {
			Control::output_func(q,state_event,yb);
			CommandSig* sig = new CommandSig(T[0],T[1]);
			IO_Type msg;
			msg.port = command;
			msg.value =
				new NetworkData(NetworkData::APP_DATA,robotAddr,100,sig);
			yb.insert(msg);
		}
		void gc_output(adevs::Bag<IO_Type>& g) {
			adevs::Bag<IO_Type>::iterator iter = g.begin();
			for (; iter != g.end(); iter++)
				delete (*iter).value;
		}

	private:
		bool doCmd;
		double err[2], ierr[2], T[2];
		void process_input_data(double h, const Bag<IO_Type>& xb) {
			for (Bag<IO_Type>::const_iterator iter = xb.begin();
				iter != xb.end(); iter++)
			{
				NetworkData* pkt =
					dynamic_cast<NetworkData*>((*iter).value);
				SampleSig* sig =
					dynamic_cast<SampleSig*>(pkt->getPayload());
				process_info(sig->getQ1(),sig->getQ2(),h);
			}
		}
		void process_info(double q1, double q2, double h) {
			double oldErr[2], derr[2];
			oldErr[0] = err[0];
			oldErr[1] = err[1];
			err[0] = get_qd1()-q1;
			err[1] = get_qd2()-q2;
			for (int i = 0; i < 2; i++) {
				derr[i] = (err[i]-oldErr[i])/h;
				ierr[i] += h*err[i];
				T[i] = 2000.0*(err[i]+0.05*derr[i]+ierr[i]/300.0);
			}
			doCmd = true;
		}
};

const int ControlExt::sample = 0;
const int ControlExt::command = 1;

class RobotExt:
	public FMI<IO_Type>
{
	public:

		static const int sample;
		static const int command;

#ifdef sampled 
		double get_q1() { return get_real(39); }
		double get_q1_sample() { return get_real(45); }
		double get_q2() { return get_real(40); }
		double get_q2_sample() { return get_real(46); }
		double get_x() { return get_real(41); }
		double get_xd() { return get_real(42); }
		double get_z() { return get_real(43); }
		double get_zd() { return get_real(44); }
		double get_error() { return get_real(38); }

		void set_T(double val, int which)
		{
			if (which == 0) set_real(22,val);
			else set_real(23,val);
		}
#else
		double get_q1() { return get_real(39); }
		double get_q1_sample() { return get_real(40); }
		double get_q2() { return get_real(41); }
		double get_q2_sample() { return get_real(42); }
		double get_x() { return get_real(43); }
		double get_xd() { return get_real(44); }
		double get_z() { return get_real(45); }
		double get_zd() { return get_real(46); }
		double get_error() { return get_real(38); }

		void set_T(double val, int which)
		{
			if (which == 0) set_real(22,val);
			else set_real(23,val);
		}
#endif

		RobotExt():
			FMI<IO_Type>
			(
			 	"Robot",
#ifdef sampled
				"{8c4e810f-3df3-4a00-8276-176fa3c9f9e0}",
				4, // Number of state variables
				0, // Number of event indicators
#else
				"{8c4e810f-3df3-4a00-8276-176fa3c9f9e0}",
				4, // Number of state variables
				2, // Number of event indicators
#endif
				"robot/linux64/Robot.so"
			),
			doSample(true)
		{
		}
		double time_event_func(const double* q)
		{
			double tSup = FMI<IO_Type>::time_event_func(q);
			if (doSample) return 0.0;
			else return tSup;
		}
		void internal_event(double* q, const bool* state_event)
		{
			FMI<IO_Type>::internal_event(q,state_event);
			test_for_sample();
			FMI<IO_Type>::internal_event(q,state_event);
		}
		void external_event(double* q, double e,
			const adevs::Bag<IO_Type>& xb)
		{
			FMI<IO_Type>::external_event(q,e,xb);
			process_input_data(xb);
			FMI<IO_Type>::external_event(q,e,xb);
		}
		void confluent_event(double *q, const bool* state_event,
			const adevs::Bag<IO_Type>& xb)
		{
			FMI<IO_Type>::confluent_event(q,state_event,xb);
			test_for_sample();
			process_input_data(xb);
			FMI<IO_Type>::confluent_event(q,state_event,xb);
		}
		void output_func(const double *q, const bool* state_event,
           adevs::Bag<IO_Type>& yb)
		{
			if (doSample)
			{
				SampleSig* sig = new SampleSig(get_q1(),get_q2());	
				IO_Type msg;
				msg.port = sample;
				msg.value = new NetworkData(NetworkData::APP_DATA,controlAddr,100,sig);
				yb.insert(msg);
			}
		}
		void gc_output(adevs::Bag<IO_Type>& g)
		{
			adevs::Bag<IO_Type>::iterator iter = g.begin();
			for (; iter != g.end(); iter++)
				delete (*iter).value;
		}
	private:
		double q1_sample_value, q2_sample_value;
		bool doSample;

		void process_input_data(const Bag<IO_Type>& xb)
		{
			for (Bag<IO_Type>::const_iterator iter = xb.begin();
				iter != xb.end(); iter++)
			{
				assert((*iter).port == command);
				assert((*iter).value != NULL);
				NetworkData* pkt =
					dynamic_cast<NetworkData*>((*iter).value);
				CommandSig* sig =
					dynamic_cast<CommandSig*>(pkt->getPayload());
				process_command(sig->getT1(),sig->getT2());
			}
		}
		void process_command(double T1, double T2)
		{
			set_T(T1,0);
			set_T(T2,1);
		}
		void test_for_sample()
		{
			doSample = (q1_sample_value != get_q1_sample() || q2_sample_value != get_q2_sample());
			if (doSample)
			{
				q1_sample_value = get_q1_sample();
				q2_sample_value = get_q2_sample();
			}
		}
};

const int RobotExt::sample = 1;
const int RobotExt::command = 2;

void makeDirectNetwork(Digraph<SimObject*>* model, Devs<IO_Type>* hybrid_ctrl,
	Devs<IO_Type>* hybrid_arm)
{
	model->couple(hybrid_arm,RobotExt::sample,hybrid_ctrl,ControlExt::sample);
	model->couple(hybrid_ctrl,ControlExt::command,hybrid_arm,RobotExt::command);
}

void makeEtherNetwork(Digraph<SimObject*>* model, Devs<IO_Type>* hybrid_ctrl,
	Devs<IO_Type>* hybrid_arm)
{
	Ethernet* net = new Ethernet();
	model->add(net);
	NetworkCard* robotApp = new NetworkCard(robotAddr);
	NetworkCard* controlApp = new NetworkCard(controlAddr);
	int to_app, from_app;
	net->attach(robotApp,to_app,from_app);
	model->couple(hybrid_arm,RobotExt::sample,net,from_app);
	model->couple(net,to_app,hybrid_arm,RobotExt::command);
	net->attach(controlApp,to_app,from_app);
	model->couple(hybrid_ctrl,ControlExt::command,net,from_app);
	model->couple(net,to_app,hybrid_ctrl,ControlExt::sample);

	for (int i = 0; i < numApps; i++)
	{
		App* app = new App(125.0,100);
		model->add(app);
		NetworkCard* nicApp = new NetworkCard(-9999);
		net->attach(nicApp,to_app,from_app);
		model->couple(app,app->data_out,net,from_app);
	}
}

void print_report(RobotExt* arm, double t)
{
}

int main(int argc, char** argv)
{
	if (argc == 2)
		numApps = atoi(argv[1]);
	RobotExt* arm = new RobotExt();
	Hybrid<IO_Type>* hybrid_arm =
		new Hybrid<IO_Type>(
		arm,
		new rk_45<IO_Type>(arm,1E-5,0.001),
		new discontinuous_event_locator<IO_Type>(arm,1E-5));
	ControlExt* ctrl = new ControlExt();
	Hybrid<IO_Type>* hybrid_ctrl =
		new Hybrid<IO_Type>(
		ctrl,
		new rk_45<IO_Type>(ctrl,1E-5,0.001),
		new discontinuous_event_locator<IO_Type>(ctrl,1E-5));
	Digraph<SimObject*>* model = new Digraph<SimObject*>();
	model->add(hybrid_ctrl);
	model->add(hybrid_arm);
	makeEtherNetwork(model,hybrid_ctrl,hybrid_arm);
	// Create the simulator
	Simulator<IO_Type>* sim =
		new Simulator<IO_Type>(model);
	// Run the simulation, testing the solution as we go
	double tReport = 0.0;
	double maxError = 0.0;
	while (sim->nextEventTime() <= 20.0)
	{
		if (maxError < arm->get_error())
			maxError = arm->get_error();
		double t = sim->nextEventTime();
		sim->execNextEvent();
		if (t - tReport >= 0.001)
		{
			tReport = t;
			cout << tReport << " " << arm->get_x() << " " << arm->get_z() << " " <<
				ctrl->get_xd() << " " << ctrl->get_zd() << " " <<
				arm->get_q1() << " " << arm->get_q2() << " " <<
				arm->get_error() << " " << maxError << endl;
		} 
	}
	if (maxError < arm->get_error())
		maxError = arm->get_error();
	cerr << "L1 err = " << maxError << endl;
	delete sim;
	delete model;
	return 0;
}
