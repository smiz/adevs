/***************
Copyright (C) 2007 by James Nutaro

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Bugs, comments, and questions can be sent to nutaro@gmail.com
***************/
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
     * */
    class InterPoly
    {
	public:
	    /**
	     * Construct a polynomial to interpolate
	     * a function u(t). The u- values
	     * are the depedent variable and t- the independent
	     * variables.
	     * */
	    InterPoly(const double* u, const double* t, unsigned int n);
		/**
		 * Construct a polynomial to interpolate u(t) with data points
		 * that are regularly spaced in time from an offset t0
		 * */
		InterPoly(const double* u, double dt, unsigned int n, double t0 = 0.0);
		/**
		 * Assign new values to the data set. If t is NULL, then
		 * only new u values will be assigned and the old t data is
		 * kept.
		 * */
		void setData(const double* u, const double* t = NULL);
	    /**
	     * Get the interpolated value at t
	     * */
	    double interpolate(double t) const;
	    /**
	     * Overloaded operator for the interpolate method
	     * */
	    double operator()(double t) const;
	    /**
	     * Approximate the function derivative at t
	     * */
	    double derivative(double t) const;
	    /**
	     * Destructor
	     * */
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
