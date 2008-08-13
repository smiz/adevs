#include <cmath>
#include <iostream>
using namespace std;

/**
 * This program computes an exact solution to the bucket problem described
 * in Chapter 5 of the manual.
 */

void pline(double t, double v) {
	cout << t << " " << v << endl;
}
int main()
{
	// Solution is equal to zero in [0,1]
	pline(0.0,0.0);
	pline(0.25,0.0);
	pline(0.5,0.0);
	pline(0.75,0.0);
	pline(1.0,0.0);
	// Spigot opens 
	double t0 = 1.0;
	// Bucket fills
	for (double dt = 0.0; dt <= -log(0.25); dt+= 0.05)
	   pline(t0+dt,1.0-exp(-dt));
	// Bucket empties
	t0 = t0-log(0.25);
	pline(t0,0.75);
	pline(t0,0.0);
	// Bucket fills again
	for (double dt = 0.0; dt <= -log(0.25); dt+= 0.05)
	   pline(t0+dt,1.0-exp(-dt));
	// Bucket empties
	t0 = t0-log(0.25);
	pline(t0,0.75);
	pline(t0,0.0);
	// Bucket fills again, but is interrupted by spigot closing
	for (double dt = 0.0; dt <= -log(0.25) && t0+dt < 4.0; dt+= 0.05)
	   pline(t0+dt,1.0-exp(-dt));
	// Spigot closes
	pline(4.0,1.0-exp(t0-4.0));
	pline(4.25,1.0-exp(t0-4.0));
	pline(4.5,1.0-exp(t0-4.0));
	pline(4.75,1.0-exp(t0-4.0));
	pline(5.0,1.0-exp(t0-4.0));
	// Done
	return 0;
}
	

