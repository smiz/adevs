#include "harmonic_compensator.h"
#include "pwm.h"
#include <iostream>
#include <fftw3.h>
#include <complex>
#include <cstdlib>
#include <fstream>
using namespace std;
using namespace adevs;

/**
  * Use genetic programming (a randomized gradient descent!)
  * to find the best choice of inverter parameters C, Linv, G, and H.
  */
class Listener:
	public EventListener<event_t>
{
	public:
		Listener(int num_samples):
			EventListener<event_t>(),
			num_samples(num_samples),
			sample(0),
			time(new double[num_samples]),
			samples(new double[num_samples])
		{
		}
		double* get_samples() { return samples; }
		bool is_done() const { return sample == num_samples; }
		void outputEvent(Event<event_t> x, double t)
		{
			if (x.value.type == ABC_SAMPLE)
			{
				time[sample] = t;
				samples[sample] = x.value.value.data.iabc[0];
				sample++;
			}
		}
		void saveCurrent()
		{
			ofstream fout("soln.txt");
			for (int i = 0; i < num_samples; i++)
				fout << time[i] << " " << samples[i] << endl;
		}
		void stateChange(Atomic<event_t>*,double){}
		~Listener()
		{
			delete [] time;
			delete [] samples;
		}
	private:
		const int num_samples;
		int sample;
		double* time;
		double* samples;
};

static const double C0 = 1E-3;
static const double Linv0 = 293E-6;
static const double G0 = 0.5;
static const double H0 = 0.1;
static const int num_samples = 30000;

struct soln_t
{
	double C;
	double Linv;
	double G;
	double H;
	double score;
};

unsigned simRuns = 0;
soln_t best;

double uniform(double a, double b, unsigned* s)
{
	double u = ((double)rand_r(s))/((double)RAND_MAX);
	return a*u+(1.0-u)*b;
}

void opt()
{
	fftw_plan plan;
	complex<double>* out = new complex<double>[num_samples];
	unsigned s;
	soln_t local;
	#pragma omp critical
	{
		s = rand();
		local = best;
	}
	while (true)
	{
		local.Linv *= uniform(0.9,1.1,&s);
		local.C *= uniform(0.9,1.1,&s);
		local.G *= uniform(0.9,1.1,&s);
		local.H *= uniform(0.9,1.1,&s);
		Listener* l = new Listener(num_samples);
		// Create the model
		HarmonicCircuit* circuit = new HarmonicCircuit(local.C,local.Linv);
		HarmonicCompensator* control = new HarmonicCompensator(local.G,local.H);
		PWM* pwm = new PWM(33000.0);
		SimpleDigraph<event_t>* model = new SimpleDigraph<event_t>();
		model->add(circuit);
		model->add(control);
		model->add(pwm);
		model->couple(circuit,control);
		model->couple(control,pwm);
		model->couple(pwm,circuit);
		// Create the simulator
		Simulator<event_t>* sim = new Simulator<event_t>(model);
		sim->addEventListener(l);
		while (!l->is_done())
			sim->execNextEvent();
		delete sim;
		delete model;
		// Get the fft and calculate the objective function
		#pragma omp critical
		plan = fftw_plan_dft_r2c_1d(
				num_samples,
				l->get_samples(),
				reinterpret_cast<fftw_complex*>(out),
				FFTW_ESTIMATE);
		fftw_execute(plan);
		// Find components of interest
		double df = circuit->sample_freq()/(double)num_samples;
		int main_left = (int)ceil(60.0/df);
		int main_right = (int)floor(60.0/df);
		int har_left = (int)ceil(300.0/df);
		int har_right = (int)floor(300.0/df);
		double har_sig = abs(out[har_left])+abs(out[har_right]);
		har_sig /= (double)num_samples;
		double main_sig = abs(out[main_left])+abs(out[main_right]);
		main_sig /= (double)num_samples;
		double remainder = 0.0;
		// Add up everything else
		for (int i = 0; i < num_samples; i++)
		{
			if (i!= main_left && i != main_right)
				remainder += abs(out[i])/(double)num_samples;
		}
		local.score = har_sig+0.1*remainder-0.1*main_sig;
		#pragma omp critical
		{
			simRuns++;
			if (local.score < best.score)
			{
				best = local;
				cout << "After " << simRuns << " tries..." << endl;
				cout << "main = " << main_sig << endl;
				cout << "harmonic = " << har_sig << endl;
				cout << "the rest = " << remainder << endl;
				cout << "C = " << best.C << endl;
				cout << "Linv = " << best.Linv << endl;
				cout << "G = " << best.G << endl;
				cout << "H = " << best.H << endl;
				cout << "score = " << best.score << endl;
				l->saveCurrent();
			}
			else
				local = best;
			fftw_destroy_plan(plan);
		}	
		// Cleanup
		delete l;
	}
	delete [] out;
}

int main()
{
	best.C = C0;
	best.Linv = Linv0;
	best.G = G0;
	best.H = H0;
	best.score = DBL_MAX;
	#pragma omp parallel
	{
		opt();
	}
	return 0;
}
