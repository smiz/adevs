/**
 * Copyright (c) 2013, James Nutaro
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of the FreeBSD Project.
 *
 * Bugs, comments, and questions can be sent to nutaro@gmail.com
 */
#ifndef _ADEVS_SIMULATION_RUNTIME_H
#define _ADEVS_SIMULATION_RUNTIME_H
#include "openmodelica.h"
#include "openmodelica_func.h"
#include "adevs_public_modelica_runtime.h"
#include <cmath>
#include <kinsol/kinsol.h>
#include <kinsol/kinsol_dense.h>
#include <nvector/nvector_serial.h>
#include <sundials/sundials_types.h>
#include <sundials/sundials_math.h>

extern "C"
{
int newuoa_(
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

void MODELICA_ASSERT(const char* file, int line, const char* msg);
void MODELICA_TERMINATE(const char* msg);

#ifdef DIVISION
#undef DIVISION
#endif
#define DIVISION(a,b) (a/b)

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

class AdevsFloorFunc:
	public AdevsMathEventFunc
{
	public:
		AdevsFloorFunc(double eps):AdevsMathEventFunc(eps){}
		virtual double calcValue(double expr);
		virtual double getZUp(double expr);
		virtual double getZDown(double expr);
		virtual ~AdevsFloorFunc(){}
	private:
		double above, below, now;
};

class AdevsCeilFunc:
	public AdevsMathEventFunc
{
	public:
		AdevsCeilFunc(double eps):AdevsMathEventFunc(eps){}
		virtual double calcValue(double expr);
		virtual double getZUp(double expr);
		virtual double getZDown(double expr);
		virtual ~AdevsCeilFunc(){}
	private:
		double above, below, now;
};

class AdevsDivFunc:
	public AdevsMathEventFunc
{
	public:
		AdevsDivFunc(double eps):AdevsMathEventFunc(eps){}
		virtual double calcValue(double expr);
		virtual double getZUp(double expr);
		virtual double getZDown(double expr);
		virtual ~AdevsDivFunc(){}
	private:
		double above, below, now;
		void calc_above_below();
};

namespace adevs
{
	bool selectDynamicStates(double*,const long int,
		const long int,long int*,long int*);
}

#endif
