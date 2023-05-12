#ifndef _circuit_h_
#define _circuit_h_
#include "adevs.h"
#include "events.h"

/**
  * Model the analog electrical circuit.
  * 
  *  Inverter - Switch - Linv +  Lg + <- Is
  *                           |     |     |
  *                           C     Rs    |
  *                           |     |     |
  *                              Ground 
  * Produces current and voltage measurements
  * at a fixed sample rate. Circuit schematic.
  */
class Circuit:
	public adevs::Atomic<event_t> 
{
	public:
		/// Phase angles
		static const double phase_angle[3];

		/**
		  * Create an equation set for the circuit
		  * @param C Capacitor 
		  * @param Linv Inductance of the line from switch
		  * @param Ls Inductance of the power line
		  * @param fs Frequency of the power system voltage
		  * @param Is Amplitude of the power system current
		  * @param Rs Load impedance
		  * @param Vinv DC voltage of the inverter source
		  * @param sample_rate Frequency at which to sample line
		  * current and voltage.
		  */ 
		Circuit(
			double C,
			double Linv,
			double Ls,
			double fs,
			double Is,
			double Rs,
			double Vinv,
			double sample_rate);
		void delta_int();
		void delta_conf(const adevs::Bag<event_t>& xb);
		void delta_ext(double e, const adevs::Bag<event_t>& xb);
		double ta();
		void output_func(adevs::Bag<event_t>& yb);
		/// Unused garbage collection
		void gc_output(adevs::Bag<event_t>&){}
		/// Destructor
		~Circuit();
		/// Current contamination term to be supplied 
		virtual double current_comp(double t, int phase, double phase_angle) = 0;
		/// Main current signal
		double current_main(double t, int phase) const;
	protected:
		/// Circuit parameters
		const double C;
		const double Linv;
		const double Vinv;
		const double Ls;
		const double Fs;
		const double Is;
		const double Rs;
		/// Sample period
		const double sample_period;
	private:
		/// State transition matrix
		double A[9];
		/// Switch states
		double polarity[3];
		/// Currents and voltages (the continuous state)
		double q[2][9];
		double *qnow, *qnext;
		/// Current time
		double time;
		/// Time to next sample
		double ts;
		/// Calculate injected current
		double current_grid(double t, int phase);
		/// Trapezoidal rule integration step
		void integrate(double h);
		/// Set the switch polarities from input
		void set_switches(const adevs::Bag<event_t>& xb);

		// For lapack
		double Aleft[9];
		char TRANS;
		int LWORK, N, M, NRHS, LDA, LDB, INCX, RANK, info;
		int JPVT[3];
		double RCOND;
		double* WORK;
};

#endif

