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

#ifndef _adevs_corrected_euler_h_
#define _adevs_corrected_euler_h_

#include <algorithm>
#include <any>
#include <cmath>
#include "adevs/solvers/hybrid.h"


namespace adevs {

/**
 * @brief This is the second order accurate RK2 method with adaptive step sizing for
 * error control.
 * 
 * A second order accurate explicit numerical method for simulation piecewise continuous
 * systems. 
 * 
 * @see Hybrid
 * @see ode_system
 */
template <typename ValueType = std::any>
class corrected_euler : public ode_solver<ValueType> {
  public:
    /**
     * @brief Create an integrator that will use the specified per step error
     * tolerance and maximum step size.
     * 
     * @param sys The ode_system that provides the derivative function to be
     * integrated numerically
     * @param err_tol The per step error tolerance
     * @param h_max The maximum allowable step size in time
     */
    corrected_euler(ode_system<ValueType>* sys, double err_tol, double h_max);
    /**
     * @brief Destructor
     * 
     * The destructor leaves the ode_system intact.
     */
    ~corrected_euler();
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
        *t,                // temporary variable for computing k2
        *k[2];             // k1 and k2
    double const err_tol;  // Error tolerance
    double const h_max;    // Maximum time step
    double h_cur;          // Previous time step that satisfied error constraint
    // Compute a step of size h, put it in qq, and return the error
    double trial_step(double h);
};

template <typename ValueType>
corrected_euler<ValueType>::corrected_euler(ode_system<ValueType>* sys, double err_tol,
                                            double h_max)
    : ode_solver<ValueType>(sys), err_tol(err_tol), h_max(h_max), h_cur(h_max) {
    for (int i = 0; i < 2; i++) {
        k[i] = new double[sys->numVars()];
    }
    dq = new double[sys->numVars()];
    qq = new double[sys->numVars()];
    t = new double[sys->numVars()];
}

template <typename ValueType>
corrected_euler<ValueType>::~corrected_euler() {
    delete[] t;
    delete[] qq;
    delete[] dq;
    for (int i = 0; i < 2; i++) {
        delete[] k[i];
    }
}

template <typename ValueType>
void corrected_euler<ValueType>::advance(double* q, double h) {
    double dt;
    while ((dt = integrate(q, h)) < h) {
        h -= dt;
    }
}

template <typename ValueType>
double corrected_euler<ValueType>::integrate(double* q, double h_lim) {
    // Initial error estimate and step size
    double err = DBL_MAX, h = std::min<double>(h_cur * 1.1, std::min<double>(h_max, h_lim));
    for (;;) {
        // Copy q to the trial vector
        for (int i = 0; i < this->sys->numVars(); i++) {
            qq[i] = q[i];
        }
        // Make the trial step which will be stored in qq
        err = trial_step(h);
        // If the error is ok, then we have found the proper step size
        if (err <= err_tol) {  // Keep h if shrunk to control the error
            if (h_lim >= h_cur) {
                h_cur = h;
            }
            break;
        }
        // Otherwise shrink the step size and try again
        else {
            double h_guess = 0.8 * err_tol * h / fabs(err);
            if (h < h_guess) {
                h *= 0.8;
            } else {
                h = h_guess;
            }
        }
    }
    // Put the trial solution in q and return the selected step size
    for (int i = 0; i < this->sys->numVars(); i++) {
        q[i] = qq[i];
    }
    return h;
}

template <typename ValueType>
double corrected_euler<ValueType>::trial_step(double step) {
    int j;
    // Compute k1
    this->sys->der_func(qq, dq);
    for (j = 0; j < this->sys->numVars(); j++) {
        k[0][j] = step * dq[j];
    }
    // Compute k2
    for (j = 0; j < this->sys->numVars(); j++) {
        t[j] = qq[j] + 0.5 * k[0][j];
    }
    this->sys->der_func(t, dq);
    for (j = 0; j < this->sys->numVars(); j++) {
        k[1][j] = step * dq[j];
    }
    // Compute next state and approximate error
    double err = 0.0;
    for (j = 0; j < this->sys->numVars(); j++) {
        qq[j] += k[1][j];                                      // Next state
        err = std::max<double>(err, fabs(k[0][j] - k[1][j]));  // Maximum error
    }
    return err;  // Return the error
}

}  // namespace adevs
#endif
