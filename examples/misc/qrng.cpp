/**
 * Model of a quantum random number generator based on the photon time
 * of arrival principle.
 */
#include "adevs.h"
#include <cmath>
using namespace std;
using namespace adevs;

typedef double io_type;
static adevs::rv rand_var;

static const double theta_t = 9.16E5;
static const double t_a = 1E-6; // 1E-7; // 1E-7; 5E-8;
static const double n_s = 0.0; // 1E-8;

static const bool accept(double tau)
{
	// Old method
//	double p = exp(theta_t*t_a)-exp(theta_t*(t_a-tau));
	// New method
	double p = exp(-theta_t*t_a)-exp(-theta_t*tau)
		+ 0.5*(-exp(-theta_t*t_a)+1.0);
	return (p <= 0.5);
}

/**
 * Model of the laser and detector diode with detection parameter
 * theta_t. This model generates photon detection events.
 */
class LaserAndDetector:
	public Atomic<io_type>
{
	public:
		LaserAndDetector():
			Atomic<io_type>(),
			event(0)
		{
		}
		void delta_int()
		{
			event++;
		}
		void delta_conf(const Bag<io_type>&){}
		void delta_ext(double,const Bag<io_type>&){}
		void output_func(Bag<io_type>& y)
		{
			y.insert(double(event));
		}
		void gc_output(Bag<io_type>&){}
		double ta()
		{
			return rand_var.exponential(1.0/theta_t);
		}
	private:
		unsigned long event;
};

/**
 * Model of the detection electronics. This model accepts detection
 * events and emits reported detections.
 */
class DetectorElectronics:
	public Atomic<io_type>
{
	public:
		DetectorElectronics():
			Atomic<io_type>(),
			mode(0)
		{
		}
		void delta_int()
		{
			mode = 0;
		}
		void delta_ext(double e,const Bag<io_type>&)
		{
			tau = e+noise();
			mode = 1;
		}
		void delta_conf(const Bag<io_type>& xb)
		{
			delta_ext(ta(),xb);
		}
		void output_func(Bag<io_type>& yb)
		{
			if (tau >= t_a)
				yb.insert(tau);
		}
		void gc_output(Bag<io_type>&){}
		double ta()
		{
			if (mode == 0) return adevs_inf<double>();
			else return 0.0;
		}

	private:
		double tau;
		int mode; 

		double noise()
		{
			if (n_s > 0.0)
				return rand_var.normal(0.0,n_s);
			else return 0.0;
		}
};

/**
 * Record detection events
 */
class DetectorRecorder:
	public Atomic<io_type>
{
	public:
		DetectorRecorder():
			Atomic<io_type>(),
			t(0.0),
			count(0)
		{
		}
		void delta_int(){}
		void delta_ext(double e,const Bag<io_type>& xb)
		{
			double tau = *(xb.begin());
			t += e;
			count++;
			cout << t << " " << count << " " << tau << " " << accept(tau) << endl;
		}
		void delta_conf(const Bag<io_type>& xb){}
		void output_func(Bag<io_type>& yb){}
		void gc_output(Bag<io_type>&){}
		double ta() { return adevs_inf<double>(); }
		unsigned getCount() const { return count; }
	private:
		double t;
		unsigned count;
};

int main()
{
	SimpleDigraph<io_type>* model = new SimpleDigraph<io_type>();
	LaserAndDetector* m1 = new LaserAndDetector();
	DetectorElectronics* m2 = new DetectorElectronics();
	DetectorRecorder* m3 = new DetectorRecorder();
	model->add(m1);
	model->add(m2);
	model->add(m3);
	model->couple(m1,m2);
	model->couple(m2,m3);
	Simulator<io_type>* sim = new Simulator<io_type>(model);
	double tL = 0.0;
	while (m3->getCount() < 10000000)
	{
		tL = sim->nextEventTime();
		sim->execNextEvent();
	}
	delete sim;
	delete model;
}

