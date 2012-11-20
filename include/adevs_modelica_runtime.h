#ifndef _ADEVS_SIMULATION_RUNTIME_H
#define _ADEVS_SIMULATION_RUNTIME_H
#include "modelica.h"
#include "openmodelica.h"
#include "openmodelica_func.h"
#include "omc_error.h"
#include "adevs_public_modelica_runtime.h"
#include <kinsol/kinsol.h>
#include <kinsol/kinsol_dense.h>
#include <nvector/nvector_serial.h>
#include <sundials/sundials_types.h>
#include <sundials/sundials_math.h>

typedef FILE_INFO omc_fileInfo;

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

#define $__start(x) x

#ifdef MODELICA_TERMINATE
#undef MODELICA_TERMINATE
#endif
#ifdef MODELICA_ASSERT
#undef MODELICA_ASSERT
#endif

void MODELICA_ASSERT(omc_fileInfo fileInfo, const char* msg);
void MODELICA_TERMINATE(const char* msg);

#ifdef DIVISION
#undef DIVISION
#endif
#define DIVISION(a,b,c) (a/b)

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
