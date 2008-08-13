#include "adevs.h"
#include "ie.h"
#include <iostream>
using namespace std;
using namespace adevs;

double t = 0.0; // Global simulation time variable

// Spigot control model
class spigot_control: public Atomic<bool> {
	public:
		spigot_control():Atomic<bool>(),count(0){}
		void delta_int() { count++; }
		void delta_ext(double,const Bag<bool>&){}
		void delta_conf(const Bag<bool>&){}
		double ta() { 
			if (count == 0) return 1.0; 
			else if (count == 1) return 3.0;
			else return DBL_MAX;
		}
		void output_func(Bag<bool>& yb) { yb.insert(count == 0); }
		void gc_output(Bag<bool>&){}
	private:
		int count;
};

// The bucket model
class bucket: public ie<bool> {
	public:
		bucket():ie<bool>(0.0,0.01),spigot_open(false){}
		double der_func(double q) { return spigot_open*(1.0-q); }
		double state_event_func(double q) { return 0.75-q; }
		void discrete_action(double& q, const Bag<bool>& xb) {
			if (q >= 0.75) q = 0.0;
			if (xb.size() > 0) spigot_open = *(xb.begin());
		}
		void discrete_output(double q, Bag<bool>& yb) {
			if (q >= 0.75) yb.insert(true);
		}
		void state_changed(double q) {
			cout << t << " " << q << " " << spigot_open << endl;
		}
		void gc_output(Bag<bool>&){}
	private:
		bool spigot_open;
};

class bucket_rk4: public rk4<bool> {
	public:
		bucket_rk4():rk4<bool>(1,0.01),spigot_open(false){ init(0,0.0); }
		void der_func(const double* q, double* dq) {
			dq[0] = spigot_open*(1.0-q[0]);
		}
		void state_event_func(const double* q, double* z) {
			z[0] = 0.75-q[0]; 
		}
		void discrete_action(double* q, const Bag<bool>& xb) {
			if (q[0] >= 0.75) q[0] = 0.0;
			if (xb.size() > 0) spigot_open = *(xb.begin());
		}
		void discrete_output(const double* q, Bag<bool>& yb) {
			if (q[0] >= 0.75) yb.insert(true);
		}
		void state_changed(const double* q) {
			cout << ::t << " " << q[0] << " " << spigot_open << endl;
		}
		void gc_output(Bag<bool>&){}
		double time_event_func(const double*) { return DBL_MAX; }
	private:
		double spigot_open;
};

int main() {
	SimpleDigraph<bool>* model = new SimpleDigraph<bool>();
	bucket* b = new bucket();
//	bucket_rk4* b = new bucket_rk4();
	spigot_control* c = new spigot_control();
	model->add(b);
	model->add(c);
	model->couple(c,b);
	Simulator<bool>* sim = new Simulator<bool>(model);
	while (sim->nextEventTime() < 5.0) {
		t = sim->nextEventTime();
		sim->execNextEvent();
	}
	delete model; delete sim;
	return 0;
}
