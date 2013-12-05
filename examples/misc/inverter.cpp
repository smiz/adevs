#include "adevs.h"
#include <iostream>
using namespace std;
using namespace adevs;

#define VOLTAGE 0
#define CURRENT 1

const double pi = 3.1415926535897931;
typedef int IO_Type;

class inverter:
	public ode_system<IO_Type>
{
public:
	inverter():
		ode_system<IO_Type>(2,4),
		R(0.6), // Resistor in Ohms
		L(0.1), // Inductor in H
		C(0.04), // Capacitor in C
		Vdc(5.0), // DC Voltage in V
		c(1.0), // Center of reference band
		ci(0.9*c), // Inner limit of reference band
		co(1.1*c), // Outer limit of reference band
		w(100.0*pi), // Reference frequency 
//		a(1.0/sqrt(R*R+pow(L*w-1.0/(C*w),2.0))), // Shape of ref. band
		a(0.15), // Shape of ref. band
		b(a/(C*w)) // Shape of ref. band
	{
	}
	void init(double *q)
	{
		qc = -1; // Initial control selection 
		q[VOLTAGE] = 0.01252; // Initial voltage
		q[CURRENT] = 0.0; // Initial current
		qc = control(q[VOLTAGE],q[CURRENT],qc); // Calculate the initial control
	}
	void der_func(const double *q, double *dq)
	{
		// dv/dt
		dq[VOLTAGE] = q[CURRENT]/C;
		// di/dt
		dq[CURRENT] = qc*(Vdc/L)-((R/L)*q[CURRENT])-((1.0/L)*q[VOLTAGE]);
	}
	void state_event_func (const double *q, double *z)
	{
		double Vz = V(q[VOLTAGE],q[CURRENT]);
		z[0] = ci-Vz;
		z[1] = co-Vz;
		z[2] = q[VOLTAGE];
		z[3] = q[CURRENT];
	}
	void internal_event(double *q, const bool *state_event)
	{
		qc = control(q[VOLTAGE],q[CURRENT],qc);
	}
	double time_event_func(const double*) { return adevs_inf<double>(); }
	void postStep(double*){}
	void external_event(double*, double, const Bag<IO_Type>&){}
	void confluent_event(double*, const bool*, const Bag<IO_Type>&){}
	void output_func(const double*, const bool*, Bag<IO_Type>&){}
	void gc_output(Bag<IO_Type>&){}
	~inverter(){}
	int getControl() const { return qc; }
	double getFreq() const { return w/(2.0*pi); }
protected:
	const double R, L, C, Vdc, c, ci, co, w, a, b;
	double V(double v, double i) const
	{
		return ((i/a)*(i/a)+(v/b)*(v/b));
	}
	virtual int control(double v, double i, int q) = 0;
private:
	int qc;
};

class inverter_with_control:
	public inverter
{
public:
	inverter_with_control():inverter(),eps(1E-5){}
	~inverter_with_control(){}
private:
	const double eps;
protected:
	int control(double v, double i, int q)
	{
		double Vz = V(v,i);
		// Rule (i)
		if (Vz > co && i > 0.0 && q == 1)
			return -1;
		// Rule (ii)
		else if (Vz > co && i < 0.0 && q == -1)
			return 1;
		// Rule (iii)
		else if (Vz <= ci && i >= 0.0 && (q == -1 || q == 0))
			return 1;
		// Rule (iv)
		else if (Vz <= ci && i <= 0.0 && (q == 1 || q == 0))
			return -1;
		// Rule (v)
		else if (Vz == co && i == 0.0 && v <= 0.0 && q == 1)
			return 0;
		// Rule (vi)
		else if (Vz == co && i == 0.0 && v >= 0.0 && q == -1)
			return 0; 
		else
			return q;
	}
};

int main()
{
	const int sim_cycles = 10;
	inverter_with_control* model = new inverter_with_control();
	const double end_time = double(sim_cycles)/model->getFreq();
	Hybrid<IO_Type>* solver = new Hybrid<IO_Type>(
		model,
		new corrected_euler<IO_Type>(model,1E-8,0.001),
		new linear_event_locator<IO_Type>(model,1E-8));
	Simulator<IO_Type>* sim = new Simulator<IO_Type>(solver);
	cout << 0.0 << " ";
	cout << solver->getState(VOLTAGE) << " " << solver->getState(CURRENT) << 
			" " << model->getControl() << endl;
	while (sim->nextEventTime() < end_time)
	{
		cout << sim->nextEventTime() << " ";
		sim->execNextEvent();
		cout << solver->getState(VOLTAGE) << " " << solver->getState(CURRENT) << 
			" " << model->getControl() << endl;
	}
	delete sim;
	delete solver;
	return 0;
}
