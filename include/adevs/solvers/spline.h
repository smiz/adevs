/*
 * Copyright (c) 2024, James Nutaro
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

#ifndef _adevs_spline_h_
#define _adevs_spline_h_


namespace adevs {

/*
 * This provides a natural spline to interpolate two points
 * in and ODE solution. It is used by the event_locator
 * to look for zero crossings of state event functions.
 */
class spline {
  public:
    spline(int const N)
        : N(N),
          a(new double[N]),
          b(new double[N]),
          c(new double[N]),
          d(new double[N]) {}

    /*
     * Create a spline that interpolates over the interval h from q0
     * to qh.
     * @param q0 The state at t = 0
     * @param dq0 The state derivative at t = 0
     * @param qh The state at t = h
     * @param dqh The state derivative at t = h
     * @param h The interval over which to interpolate
     * @param N The number of equations to interpolate
     */
    void init(double const* q0, double const* dq0, double const* qh,
              double const* dqh, double const h) {
        for (int i = 0; i < N; i++) {
            d[i] = q0[i];
            c[i] = dq0[i];
            a[i] = (2.0 / (h * h * h)) *
                   ((h / 2.0) * (dqh[i] - c[i]) - qh[i] + c[i] * h + d[i]);
            b[i] = (1.0 / (2.0 * h)) * (dqh[i] - c[i] - 3.0 * a[i] * h * h);
        }
    }

    /// Destructor
    ~spline() {
        delete[] a;
        delete[] b;
        delete[] c;
        delete[] d;
    }

    void interpolate(double* q, double h) {
        double const hh = h * h;
        double const hhh = hh * h;
        for (int i = 0; i < N; i++) {
            q[i] = a[i] * hhh + b[i] * hh + c[i] * h + d[i];
        }
    }

  private:
    // Number of equations
    int const N;
    /// Parameters for cubic ax^3+bx^2_+x+d
    double *a, *b, *c, *d;
};

}  // namespace adevs
#endif
