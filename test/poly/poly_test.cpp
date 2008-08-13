#include "adevs.h"
#include <iostream>
#include <cmath>
using namespace std;

int main(int argc, char** argv)
{
	/* plot a polynomial approx to sin(t) and d/dt sin(t) = cos(t) */
	unsigned n = atoi(argv[1]);
	double tend = 6.14;
	double dt = tend/(double)n;
	double* tdat = new double[n];
	double* xdat = new double[n];
	for (unsigned i = 0; i < n; i++)
	{
		xdat[i] = sin(i*dt);
		tdat[i] = i*dt;
	}
	adevs::InterPoly p(xdat,tdat,n);
	for (double t = 0.0; t <= tend; t += dt/2.0)
	{
		cout << t << " " << p(t) << " " << p.derivative(t) << endl;
	}
	delete [] tdat;
	delete [] xdat;
	return 0;
}
