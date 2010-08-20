#ifndef _AdmittanceNetwork_h_
#define _AdmittanceNetwork_h_
#include "events.h"

/**
 * This is the admittance matrix for the transmission network.
 * It can be used to solve for current given the voltage or 
 * vice versa.
 */
class AdmittanceNetwork
{
	public:
		// Create an N bus network
		AdmittanceNetwork(int N);
		// Add a line from i to j 
		void add_line(int i, int j, Complex y);
		// Remove the line from i to j
		void remove_line(int i, int j, Complex y) { add_line(i,j,-y); }
		// Add self-impedence at node i
		void add_self(int i, Complex y);
		// Remove a self-impedence at node i
		void remove_self(int i, Complex y) { add_self(i,-y); }
		// Solve for voltage given a current
		void solve_for_voltage(const Complex* I, Complex* V);
		// Solve for current given a voltage
		void solve_for_current(const Complex* V, Complex* I);
		// Get the value at entry i,j
		Complex get(int i, int j) const;
		// Destructor
		~AdmittanceNetwork();
	private:
		// The admittance network stored for LAPACK
		const int N;
		double *Y, *LUY;
		bool dirty;	
		// Scratch space for the solvers
		double *iv_lapack;
		int* ipiv_lapack;
		void set(int i, int j, Complex y);
};

#endif
