#include "adevs.h"
#include "adevs_rk45.h"
#include "sampler.h"
#include <iostream>
#include <cmath>
using namespace std;
using namespace adevs;

static const double PI = 3.1415926535897931;

/**
Simulates a ball bouncing between two surfaces.
*/
class bouncing_ball: public rk45<PortValue<double> >
{
	public:
		bouncing_ball():
		rk45<PortValue<double> >(4,0.01,1E-5,2)
		{
			init(0,PI/2.0); // x
			init(1,2.0); // y
			init(2,0.0); // x velocity
			init(3,4.0); // y velocity
			sample = false;
		}
		void der_func(const double* q, double *dq)
		{
			dq[0] = q[2];
			dq[1] = q[3];
			dq[2] = 0.0;
			dq[3] = -9.8;
		}
		void state_event_func(const double *q, double* z)
		{
			z[0] = q[1] - cos(q[0]);
			z[1] = 2.5 - q[1];
		}
		double time_event_func(const double* q)
		{
			if (sample) return 0.0;
			else return DBL_MAX;
		}
		void discrete_action(double* q, const Bag<PortValue<double> >& xb)
		{
			if (q[1] <= cos(q[0]))
			{
				// Normalise the velocity vector
				double v[2];
				double V = sqrt(q[3]*q[3]+q[2]*q[2]);
				v[0] = q[2]/V;
				v[1] = q[3]/V;
				// Normalize the surface tangent vector
				double s[2];
				s[0] = 1.0;
				s[1] = -sin(q[0]);
				double S = sqrt(s[0]*s[0]+s[1]*s[1]);
				s[0] = s[0]/S;
				s[1] = s[1]/S;
				// Compute the dot product
				double d = v[0]*s[0]+v[1]*s[1];
				// Find the angle between v and s
				double theta = acos(d);
				theta = PI/2.0-theta;
				q[2] = (-v[0]*cos(theta)-v[1]*sin(theta))*V;
				q[3] = (v[0]*sin(theta)-v[1]*cos(theta))*V;
			}
			else if (q[1] >= 2.5)
			{
				// Hit the ceiling, so change the sign of the y velocity
				q[3] = -q[3];
			}
			if (xb.size() > 0) sample = true;
			else sample = false;
		}
		void discrete_output(const double* q, Bag<PortValue<double> >& yb)
		{
			PortValue<double> x(0,q[0]);
			yb.insert(x);
			PortValue<double> y(0,q[1]);
			yb.insert(y);
		}
		void gc_output(Bag<PortValue<double> >& g){}

	private:
		bool sample;
};

int main()
{
	bouncing_ball* ball = new bouncing_ball();
	sampler* sample = new sampler(0.01);
	Digraph<double>* model = new Digraph<double>();
	model->add(ball);
	model->add(sample);
	model->couple(sample,0,ball,0);
	model->couple(ball,0,sample,0);
	Simulator<PortValue<double> >* sim = new Simulator<PortValue<double> >(model);
	while (sim->nextEventTime() < 10.0)
	{
		sim->execNextEvent();
	}
	delete sim;
	delete model;
	return 0;
}
