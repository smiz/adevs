#ifndef _ADEVS_SIMULATION_RUNTIME_H
#define _ADEVS_SIMULATION_RUNTIME_H
#include "simulation_varinfo.h"

extern int LOG_STATS;
extern int LOG_INIT;
extern int LOG_RES_INIT;
extern int LOG_SOLVER;
extern int LOG_EVENTS;
extern int LOG_NONLIN_SYS;
extern int LOG_ZEROCROSSINGS;
extern int LOG_DEBUG;
extern int sim_verbose;
extern int modelErrorCode;
extern int ERROR_NONLINSYS;
extern int ERROR_LINSYS;

extern "C"
{
void newuoa_(
  long *nz,
  long *NPT,
  double *z,
  double *RHOBEG,
  double *RHOEND,
  long *IPRINT,
  long *MAXFUN,
  double *W,
  void (*leastSquare) (long *nz, double *z, double *funcValue)
  );
};

void MODELICA_ASSERT(omc_fileInfo info, const char* msg);
void MODELICA_TERMINATE(const char* msg);

#endif

