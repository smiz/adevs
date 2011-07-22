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

int LOG_STATS = 1;
int LOG_INIT = 1;
int LOG_RES_INIT = 1;
int LOG_SOLVER = 1;
int LOG_EVENTS = 1;
int LOG_NONLIN_SYS = 1;
int LOG_ZEROCROSSINGS = 1;
int LOG_DEBUG = 1;
int sim_verbose = 0;
int modelErrorCode = 0;
int ERROR_NONLINSYS = 1;
int ERROR_LINSYS = 2;

void MODELICA_TERMINATE(const char* msg)
{
	cerr << msg << endl;
	exit(-2);
}

void MODELICA_ASSERT(omc_fileInfo info, const char* msg)
{
	cerr << "ASSERT failed" << endl;
	cerr << "File = " << info.filename << endl;
	cerr << "Cols = " << info.colStart << " -> " << info.colEnd << endl;
	cerr << "Lns = " << info.lineStart << " -> " << info.lineEnd << endl;
	cerr << msg << endl;
	exit(-1);
}	
