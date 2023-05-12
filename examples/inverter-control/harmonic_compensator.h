#ifndef _harmonic_compensator
#define _harominc_compensator
#include "events.h"
#include "control.h"
#include "circuit.h"

/**
  * A circuit with a single harmonic at 300 Hz.
  */
class HarmonicCircuit:
	public Circuit
{
	public:
		HarmonicCircuit(double C, double Linv);
		double current_comp(double t ,int phase, double angle);
		double sample_freq() const;
};

/**
  * Drive the inverter to compensate for a single harmonic.
  */
class HarmonicCompensator:
	public Control
{
	public:
		/**
		  * Create a compensator with the given gains
		  * @param G proportional gain
        * @param H integral gain
		  */ 
		HarmonicCompensator(double G, double H);
	protected:
		void control_law(double h, const double* iabc, const double* vabc);
	private:
		const double G, H;
		const int num_samples;
		int sample_idx;
		bool sample_full;
		double* samples[3];
		const double f;
		const double Vmax;
		double e[3]; // Integral component of PI control
		double t; // Time
};

#endif
