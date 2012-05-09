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

void MODELICA_ASSERT(omc_fileInfo fileInfo, const char* msg)
{
    cerr << fileInfo.filename <<
		"(col " << fileInfo.colStart << "-" << fileInfo.colEnd << ","
		<< "ln " << fileInfo.lineStart << "-" << fileInfo.lineEnd << ")"
		<< endl;
	cerr << msg << endl;
}

void solve_linear_equation_system(double* A, double* b, int size, int id)
{
	int n = size; 
	int nrhs = 1; /* number of righthand sides*/ 
	int lda = n; /* Leading dimension of A */
	int ldb = n; /* Leading dimension of b*/ 
	int ipiv[n]; /* Pivott indices */ 
	int info = 0; /* output */ 
	assert(ipiv != 0); 
	_omc_dgesv_(&n,&nrhs,&A[0],&lda,ipiv,&b[0],&ldb,&info); 
 	if (info < 0)
	{ 
		cerr << "Error solving linear system of equations (no. " << id <<
		   	") . Argument " << info << "illegal." << endl; 
	} 
	else if (info > 0)
	{ 
		cerr << "Error solving linear system of equations (no. " << id <<
			"), system is singular." << endl; 
	}
	assert(info == 0);
}	
