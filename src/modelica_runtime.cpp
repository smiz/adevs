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
	maxDelay(maxDelay)
{
}

double AdevsDelayData::sample(double t)
{
	assert(t <= traj.back().t);
	if (t <= traj.front().t)
		return traj.front().v;
	// Find two points that bracket t
	list<point_t>::iterator p1, p2 = traj.begin();
	while ((*p2).t <= t)
	{
		p1 = p2;
		p2++;
	}
	assert((*p1).t < t);
	assert((*p2).t >= t);
	double h = (t-((*p1).t))/((*p2).t) - ((*p1).t);
	return h*((*p2).v)+(1.0-h)*((*p1).v);
}

void AdevsDelayData::insert(double t, double v)
{
	point_t p;
	p.t = t;
	p.v = v;
	assert(traj.empty() || p.t >= traj.back().t);
	if (!traj.empty() &&
		(traj.back().t - traj.front().t > maxDelay) &&
		(t - traj.front().t > maxDelay))
	{
		traj.pop_front();
	}
	traj.push_back(p);
}
