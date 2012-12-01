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

AdevsSampleData::AdevsSampleData(double tStart, double tInterval):
	tStart(tStart),
	tInterval(tInterval),
	n(0)
{
}

bool AdevsSampleData::atEvent(double tNow, double eps) const
{
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

