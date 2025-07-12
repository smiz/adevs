/*
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

#ifndef _adevs_rk_45_h_
#define _adevs_rk_45_h_

#include <any>
#include <cmath>
#include "adevs/solvers/hybrid.h"


namespace adevs {

/**
 * @brief This ode_solver implements a 4th/5th order integrator that adjust
 * its step size to control error.
 *
 * A fifth order accurate numerical integration method for simulating piecewise
 * continuous systems.Atomic
 * 
 * @see ode_system
 * @see Hybrid
 */
template <typename ValueType = std::any>
class rk_45 : public ode_solver<ValueType> {
  public:
    /**
     * @brief Constructor
     * 
     * An explicit numerical integration method. The integrator
     * will adjust its step size to maintain a per
     * step error less than err_tol, and will use a step size
     * strictly less than h_max.
     * 
     * @param sys The ode_system whose der_func() method is to be integrated
     * @param err_tol The maximum allowable per step error
     * @param h_max The largest allowable step size
     */
    rk_45(ode_system<ValueType>* sys, double err_tol, double h_max);
    /**
     * @brief Destructor
     * 
     * Leaves the supplied ode_system intact
     */
    ~rk_45();
     /**
     * @brief Integrate up to h_lim.
     * 
     * Used by a Hybrid object to simulate the system.advance
     * 
     * @param q The state at the start of a step. This is overwritten
     * with the state at the end of the integration step.
     * @param h_lim The maximum step size
     * @return The step actually taken.
     */
    double integrate(double* q, double h_lim);
     /**
     * @brief Integrate to exactly the step h
     * 
     * As with integrate() but advance the step by exactly the
     * specified step size.
     *
     * @param q The state at the start of a step. This is overwritten
     * with the state at the end of the integration step.
     * @param h The step in time by which to advance the solution
     */
    void advance(double* q, double h);

  private:
    double *dq,            // derivative
        *qq,               // trial solution
        *t,                // temporary variables for computing stages
        *k[6];             // the six RK stages
    double const err_tol;  // Error tolerance
    double const h_max;    // Maximum time step
    double h_cur;          // Previous successful step size
    // Compute a trial step of size h, store the result in qq, and return the error
    double trial_step(double h);
};

template <typename ValueType>
rk_45<ValueType>::rk_45(ode_system<ValueType>* sys, double err_tol,
                        double h_max)
    : ode_solver<ValueType>(sys), err_tol(err_tol), h_max(h_max), h_cur(h_max) {
    for (int i = 0; i < 6; i++) {
        k[i] = new double[sys->numVars()];
    }
    dq = new double[sys->numVars()];
    qq = new double[sys->numVars()];
    t = new double[sys->numVars()];
}

template <typename ValueType>
rk_45<ValueType>::~rk_45() {
    delete[] dq;
    delete[] t;
    for (int i = 0; i < 6; i++) {
        delete[] k[i];
    }
}

template <typename ValueType>
void rk_45<ValueType>::advance(double* q, double h) {
    double dt;
    while ((dt = integrate(q, h)) < h) {
        h -= dt;
    }
}

template <typename ValueType>
double rk_45<ValueType>::integrate(double* q, double h_lim) {
    // Initial error estimate and step size
    double err = DBL_MAX,
           h = std::min<double>(h_cur * 1.1, std::min<double>(h_max, h_lim));
    for (;;) {
        // Copy q to the trial vector
        for (int i = 0; i < this->sys->numVars(); i++) {
            qq[i] = q[i];
        }
        // Make the trial step which will be stored in qq
        err = trial_step(h);
        // If the error is ok, then we have found the proper step size
        if (err <= err_tol) {
            if (h_cur <= h_lim) {
                h_cur = h;
            }
            break;
        }
        // Otherwise shrink the step size and try again
        else {
            double h_guess = 0.8 * pow(err_tol * pow(h, 4.0) / fabs(err), 0.25);
            if (h < h_guess) {
                h *= 0.8;
            } else {
                h = h_guess;
            }
        }
    }
    // Copy the trial solution to q and return the step size that was selected
    for (int i = 0; i < this->sys->numVars(); i++) {
        q[i] = qq[i];
    }
    return h;
}

template <typename ValueType>
double rk_45<ValueType>::trial_step(double step) {
    // Compute k1
    this->sys->der_func(qq, dq);
    for (int j = 0; j < this->sys->numVars(); j++) {
        k[0][j] = step * dq[j];
    }
    // Compute k2
    for (int j = 0; j < this->sys->numVars(); j++) {
        t[j] = qq[j] + 0.5 * k[0][j];
    }
    this->sys->der_func(t, dq);
    for (int j = 0; j < this->sys->numVars(); j++) {
        k[1][j] = step * dq[j];
    }
    // Compute k3
    for (int j = 0; j < this->sys->numVars(); j++) {
        t[j] = qq[j] + 0.25 * (k[0][j] + k[1][j]);
    }
    this->sys->der_func(t, dq);
    for (int j = 0; j < this->sys->numVars(); j++) {
        k[2][j] = step * dq[j];
    }
    // Compute k4
    for (int j = 0; j < this->sys->numVars(); j++) {
        t[j] = qq[j] - k[1][j] + 2.0 * k[2][j];
    }
    this->sys->der_func(t, dq);
    for (int j = 0; j < this->sys->numVars(); j++) {
        k[3][j] = step * dq[j];
    }
    // Compute k5
    for (int j = 0; j < this->sys->numVars(); j++) {
        t[j] = qq[j] + (7.0 / 27.0) * k[0][j] + (10.0 / 27.0) * k[1][j] +
               (1.0 / 27.0) * k[3][j];
    }
    this->sys->der_func(t, dq);
    for (int j = 0; j < this->sys->numVars(); j++) {
        k[4][j] = step * dq[j];
    }
    // Compute k6
    for (int j = 0; j < this->sys->numVars(); j++) {
        t[j] = qq[j] + (28.0 / 625.0) * k[0][j] - 0.2 * k[1][j] +
               (546.0 / 625.0) * k[2][j] + (54.0 / 625.0) * k[3][j] -
               (378.0 / 625.0) * k[4][j];
    }
    this->sys->der_func(t, dq);
    for (int j = 0; j < this->sys->numVars(); j++) {
        k[5][j] = step * dq[j];
    }
    // Compute next state and the approximate error
    double err = 0.0;
    for (int j = 0; j < this->sys->numVars(); j++) {
        // Next state
        qq[j] += (1.0 / 24.0) * k[0][j] + (5.0 / 48.0) * k[3][j] +
                 (27.0 / 56.0) * k[4][j] + (125.0 / 336.0) * k[5][j];
        // Component wise maximum of the approximate error
        err = std::max(
            err, fabs(k[0][j] / 8.0 + 2.0 * k[2][j] / 3.0 + k[3][j] / 16.0 -
                      27.0 * k[4][j] / 56.0 - 125.0 * k[5][j] / 336.0));
    }
    // Return the error
    return err;
}

}  // namespace adevs
#endif
