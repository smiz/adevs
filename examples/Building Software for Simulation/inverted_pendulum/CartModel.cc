#include "CartModel.h"
using namespace std;
using namespace adevs;

CartModel::CartModel():
	ode_system<double>(5,2), // five cont. states, two event surfaces
	x(0), // position index
	theta(1), // arm angle  index
	dx(2), // cart velocity index
	dtheta(3), // arm angular velocity index
	t(4), // time
	armMass(1.0), // mass of the arm in kilograms
	armFric(1E-4), // resistance to rotation
	cartMass(1.0), // mass of the cart in kilograms
	cartFric(1E-4), // resistance to lateral motion
	armLen(0.5), // length of the arm in meters
	g(9.8), // acc. due to gravity in meters/second^2
	mAngle(2.0*PI/1024.0), // sensor thresholds
	k(0)
{
	k = 0; // Arm angle is initially zero
	F = 0.0; // No initial control force
	// Compute the entries of the constraint matrix that are fixed
	A[0][0] = cartMass+armMass;
	A[1][1] = armLen*armLen*armMass/4.0;
}

void CartModel::init(double* q)
{
	q[x] = 0.0; // Start at the middle of the track
	q[theta] = k*mAngle;
	q[dx] = q[dtheta] = 0.0; // No motion
	q[t] = 0.0;
}

void CartModel::der_func(const double* q, double* dq)
{
	dq[t] = 1.0; // Time
	dq[x] = q[dx]; // Velocities
	dq[theta] = q[dtheta];
	// Compute the constraint matrices
	double Fnudge = 10.0*exp(-20.0*q[t]);
	A[1][0] = A[0][1] = -armLen*armMass*cos(q[theta])/2.0;
	B[0] = (F+Fnudge)
		- armMass*armLen*q[dtheta]*q[dtheta]*sin(q[theta])/2.0
		- cartFric*q[dx];
	B[1] = armMass*armLen*g*sin(q[theta])/2.0 - armFric*q[dtheta];
	// Compute determinant of the constraint matrix
	double det = A[0][0]*A[1][1]-A[0][1]*A[1][0];
	// Solve for the accelerations
	dq[dx] = (A[1][1]*B[0]-A[0][1]*B[1])/det;
	dq[dtheta] = (A[0][0]*B[1]-A[1][0]*B[0])/det;
}

void CartModel::state_event_func(const double* q, double* z)
{
	z[0] = q[theta] - mAngle*(k-1);
	z[1] = q[theta] - mAngle*(k+1);
}

void CartModel::internal_event(double *q, const bool *event_flags)
{
	if (event_flags[0]) k--;
	else k++;
}

void CartModel::external_event(double* q, double e, const Bag<double>& xb)
{
	F = *(xb.begin());
}

void CartModel::confluent_event(double* q, const bool * event_flags,
		const Bag<double>& xb)
{
	internal_event(q,event_flags);
	external_event(q,0.0,xb);
}

void CartModel::output_func(const double *q, const bool *event_flags,
		Bag<double> &yb)
{
	if (event_flags[0]) yb.insert((k-1)*mAngle);
	else yb.insert((k+1)*mAngle);
}
