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
#ifndef _adevs_poly_h_
#define _adevs_poly_h_
#include <cstdlib>

namespace adevs
{
    /**
     * This class implements Lagrange interpolating
     * polynomials of arbitrary order for functions of
     * a single variable. This can be 
     * useful in many discrete event simulations where
     * tabular values, possible produced by discrete time
     * calculations, need to be interpolated for use
     * in a discrete event system. GDEVS is one particular
     * example of this.
     */
    class InterPoly
    {
	public:
	    /**
	     * Construct a polynomial to interpolate
	     * a function u(t). The u- values
	     * are the dependent variable and t- the independent
	     * variables.
	     */
	    InterPoly(const double* u, const double* t, unsigned int n);
		/**
		 * Construct a polynomial to interpolate u(t) with data points
		 * that are regularly spaced in time from an offset t0
		 */
		InterPoly(const double* u, double dt, unsigned int n, double t0 = 0.0);
		/**
		 * Assign new values to the data set. If t is NULL, then
		 * only new u values will be assigned and the old t data is
		 * kept.
		 */
		void setData(const double* u, const double* t = NULL);
	    /**
	     * Get the interpolated value at t
	     */
	    double interpolate(double t) const;
	    /**
	     * Overloaded operator for the interpolate method
	     */
	    double operator()(double t) const;
	    /**
	     * Approximate the function derivative at t
	     */
	    double derivative(double t) const;
	    /**
	     * Destructor
	     */
	    ~InterPoly();
	private:
	    InterPoly(){}
	    InterPoly(const InterPoly&){}
	    void operator=(const InterPoly&){}

	    double* tdat;
	    double* udat;
	    unsigned int n;
    };
}

#endif
