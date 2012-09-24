#ifndef _ADEVS_SIMULATION_RUNTIME_H
#define _ADEVS_SIMULATION_RUNTIME_H
#include "modelica.h"
#include "openmodelica.h"
#include "openmodelica_func.h"
#include "omc_error.h"
#include "adevs_public_modelica_runtime.h"

typedef FILE_INFO omc_fileInfo;
typedef EQUATION_INFO omc_equationInfo;

extern int modelErrorCode;

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
}

#ifdef MODELICA_TERMINATE
#undef MODELICA_TERMINATE
#endif
#ifdef MODELICA_ASSERT
#undef MODELICA_ASSERT
#endif

void MODELICA_ASSERT(omc_fileInfo fileInfo, const char* msg);
void MODELICA_TERMINATE(const char* msg);

#define $__start(x) x
#ifdef DIVISION
#undef DIVISION
#endif
#define DIVISION(a,b,c) (a/b)

#ifdef extraPolate
#undef extraPolate
#define extraPolate(v) v
#endif

#ifdef check_discrete_values
#undef check_discrete_values
#define check_discrete_values(size,numValues) found_solution = 1
#endif

// undefine initial() (orig. defn. in omc's simulation_runtime.h)
#ifdef initial
#undef initial
#endif
// Less, Greater, etc are positive if true, negative if false
#define Adevs_Less(exp1,exp2) ((exp2)-(exp1))
#define Adevs_Greater(exp1,exp2) ((exp1)-(exp2))
#define Adevs_LessEq(exp1,exp2) ((exp2)-(exp1))
#define Adevs_GreaterEq(exp1,exp2) ((exp1)-(exp2))
// True to false transitions occur at zero,
// False to true transitions at positive epsilon
#define ADEVS_ZEROCROSSING(index,expr) z[index] = expr-(!zc[index])*epsilon
#define ADEVS_RELATIONTOZC(var,exp1,exp2,index,OpSym) \
    if (0 <= index) { \
        if (zc[index] == -1) zc[index]=((exp1) OpSym (exp2)); \
        var=zc[index]; \
    } else var=((exp1) OpSym (exp2))
#define ADEVS_SAVEZEROCROSS(var,exp1,exp2,index,OpSym) \
    if (0 <= index) { \
        if (zc[index] == -1) zc[index]=((exp1) OpSym (exp2)); \
        var=zc[index]; \
    } else var=((exp1) OpSym (exp2))

#endif
