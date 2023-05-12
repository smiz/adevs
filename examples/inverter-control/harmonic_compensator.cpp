#include "harmonic_compensator.h"
using namespace std;
using namespace adevs;

#define SAMPLE_RATE 3000.0
#define HARMONIC_FREQ 300.0
#define VIMAX 480.0

/**
  * A circuit with a fifth harmonic of the primary frequency.
  */

HarmonicCircuit::HarmonicCircuit(
	double C,
	double Linv):
	Circuit(
		C, // C
		Linv, // Linv
		100E-6, // Ls
		60.0, // fs
		VIMAX, // Is
		1.0, // Rs
		VIMAX, // Vinv
		SAMPLE_RATE) // Sample rate
{
}	

double HarmonicCircuit::current_comp(double t, int phase, double angle)
{
	// Fifth harmonic 
	return 0.1*Is*cos(2*M_PI*HARMONIC_FREQ*t+angle);
}

double HarmonicCircuit::sample_freq() const
{
	return SAMPLE_RATE;
}

HarmonicCompensator::HarmonicCompensator(double G, double H):
	Control(),
	G(G),
	H(H),
	// Sample ten full periods of signal we want to filter
	num_samples((int)(SAMPLE_RATE*10.0/HARMONIC_FREQ)),
	sample_idx(0),
	sample_full(false),
	f(60.0),
	Vmax(VIMAX),
	t(0.0)
{
	for (int i = 0; i < 3; i++)
	{
		samples[i] = new double[num_samples];
		e[0] = 0.0;
	}
}

void HarmonicCompensator::control_law(double h, const double* iabc, const double* vabc)
{
	double ref[3];
	double idq[3];
	double cdq[3];
	t += h;
	abc_to_dq(iabc,idq,2.0*M_PI*f*t);
	for (int i = 0; i < 3; i++)
		samples[i][sample_idx] = idq[i];
	sample_idx = (sample_idx+1)%num_samples;
	if (sample_idx == 0) sample_full = true;
	if (!sample_full)
	{
		vout[0] = vout[1] = vout[2] = 0.0;
		return;
	}
	for (int i = 0; i < 3; i++)
	{
		ref[i] = samples[i][0];
		for (int k = 1; k < num_samples; k++)
			ref[i] += samples[i][k];
		ref[i] /= (double)num_samples;
		double err = ref[i]-idq[i];
		e[i] += h*err;
		cdq[i] = -(G*err+H*e[i]);
	}
	dq_to_abc(cdq,vout,2.0*M_PI*f*t);
	for (int i = 0; i < 3; i++)
	{
		vout[i] /= Vmax;
		if (vout[i] > 1.0) vout[i] = 1.0;
		if (vout[i] < -1.0) vout[i] = -1.0;
	}
}

