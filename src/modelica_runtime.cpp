/**
 * This file contains definitions of external variable types
 * and other parts of the Open Modelica libsim that cannot
 * be linked to by an adevs module. The libsim runtime seems
 * to take the role of the main program block, with main, the
 * dassl code, its links to dassl specific functions, etc.
 * that are not present in the adevs simulation runtime.
 * Whatever is needed from the library for the adevs modules
 * produced by the modelica compiler is therefore put here.
 *
 */
#include "adevs_modelica_runtime.h"
#include <iostream>
#include <cstdlib>
using namespace std;

int modelErrorCode;

void MODELICA_TERMINATE(const char* msg)
{
	cerr << msg << endl;
	exit(-2);
}

void MODELICA_ASSERT(omc_fileInfo fileInfo, const char* msg)
{
    cerr << fileInfo.filename <<
		"(col " << fileInfo.colStart << "-" << fileInfo.colEnd << ","
		<< "ln " << fileInfo.lineStart << "-" << fileInfo.lineEnd << ")"
		<< endl;
	cerr << msg << endl;
}

/**
 * Implementation of the AdevsSampleData class.
 */
AdevsSampleData::AdevsSampleData(double tStart, double tInterval):
	tStart(tStart),
	tInterval(tInterval),
	n(0),
	enabled(false)
{
}

bool AdevsSampleData::atEvent(double tNow, double eps) const
{
	if (!enabled) return false;
	double tEvent = tStart+double(n)*tInterval;
	return fabs(tEvent-tNow) < eps;
}

double AdevsSampleData::timeToEvent(double tNow) const
{
	double tEvent = tStart+double(n)*tInterval;
	double tToGo = tEvent-tNow;
	if (tToGo < 0.0) return 0.0;
	return tToGo;
}

void AdevsSampleData::update(double tNow, double eps)
{
	if (atEvent(tNow,eps))
		n++;
}

/**
 * Implementation of the AdevsDelayData class.
 */
AdevsDelayData::AdevsDelayData(double maxDelay):
	maxDelay(maxDelay),
	start(0),
	num_els(0),
	size(1024)
{
	traj = new point_t[size];
}

double AdevsDelayData::sample(double t)
{
	assert(t <= traj[get_index(num_els-1)].t);
	if (t <= traj[get_index(0)].t)
		return traj[get_index(0)].v;
	// Find two points that bracket t
	int i = 0, j = num_els-1, k;
	while (i+1 != j)
	{
		k = (i+j)/2;	
		if (t < traj[get_index(k)].t)
			j = k;
		else i = k;
	}
	assert(i < j);
	assert(i >= 0);
	assert(j < num_els);
	point_t &p2 = traj[get_index(j)];
	point_t &p1 = traj[get_index(i)];
	assert(t < p2.t);
	assert(p1.t <= t);
	double h = (t-p1.t)/(p2.t-p1.t);
	return h*p2.v+(1.0-h)*p1.v;
}

void AdevsDelayData::insert(double t, double v)
{
	point_t p;
	p.t = t;
	p.v = v;
	assert(num_els == 0 || p.t >= traj[get_index(num_els-1)].t);
	if (num_els > 1 && (t - traj[get_index(1)].t > maxDelay))
	{
		// Remove the outdated element
		num_els--;
		start = (start+1)%size;
	}
	// Expand the array if needed
	if (num_els + 1 == size)
	{
		int new_size = size*2;
		point_t* tmp = new point_t[new_size];
		for (int i = 0; i < num_els; i++)
			tmp[i] = traj[get_index(i)];
		start = 0;
		size = new_size;
		delete [] traj;
		traj = tmp;
	}
	// Add the new element
	traj[get_index(num_els)] = p;
	num_els++;
}

AdevsDelayData::~AdevsDelayData()
{
	delete [] traj;
}

/*
 * Implementation of the floor function.
 */
double AdevsFloorFunc::calcValue(double expr)
{
	if (isInInit())
	{
		now = floor(expr);
		below = now - eps;
		above = now + 1.0;
	}
	return now;
}

double AdevsFloorFunc::getZUp(double expr)
{
	return above-expr;
}

double AdevsFloorFunc::getZDown(double expr)
{
	return expr-below;
}

/*
 * Implementation of the ceiling function.
 */
double AdevsCeilFunc::calcValue(double expr)
{
	if (isInInit())
	{
		now = ceil(expr);
		above = now + eps;
		below = now - 1.0;
	}
	return now;
}

double AdevsCeilFunc::getZUp(double expr)
{
	return above-expr;
}

double AdevsCeilFunc::getZDown(double expr)
{
	return expr-below;
}

/*
 * Implementation of the div function.
 */
double AdevsDivFunc::calcValue(double expr)
{
	if (isInInit())
	{
		now = trunc(expr);
		calc_above_below();
	}
	return now;
}

void AdevsDivFunc::calc_above_below()
{
	if (now >= 1.0)
	{
		above = now + 1.0;
		below = now - eps;
	}
	else if (now <= -1.0)
	{
		above = now + eps;
		below = now - 1.0;
	}
	else // now == 0.0
	{
		above = 1.0;
		below = -1.0;
	}
}

double AdevsDivFunc::getZUp(double expr)
{
	return above-expr;
}

double AdevsDivFunc::getZDown(double expr)
{
	return expr-below;
}
