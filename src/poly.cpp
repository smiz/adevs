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
#include "adevs_poly.h"
using namespace std;
using namespace adevs;

void adevs::InterPoly::setData(const double* u, const double* t)
{
	for (unsigned int i = 0; i < n; i++)
	{
		if (t != NULL) tdat[i] = t[i];
		udat[i] = u[i];
	}
}

adevs::InterPoly::InterPoly(const double* u, const double* t, unsigned int n)
{
	this->n = n;
    tdat = new double[n];
    udat = new double[n];
    for (unsigned i = 0; i < n; i++)
    {
		tdat[i] = t[i];
		udat[i] = u[i];
    }
}

adevs::InterPoly::InterPoly(const double* u, double dt, unsigned int n, double t0)
{
	this->n = n;
    tdat = new double[n];
    udat = new double[n];
    for (unsigned  i = 0; i < n; i++)
    {
		tdat[i] = t0+(double)i*dt;
		udat[i] = u[i];
    }
}

double adevs::InterPoly::interpolate(double t) const
{
    double result = 0.0;
    for (unsigned k = 0; k < n; k++)
    {
        double l = 1.0;
        for (unsigned i = 0; i < n; i++)
        {
            if (i != k)
            {
                l *= (t-tdat[i])/(tdat[k]-tdat[i]);
            }
        }
        result += l*udat[k];
    }
    return result;
}

double adevs::InterPoly::operator()(double t) const
{
    return interpolate(t);
}

double adevs::InterPoly::derivative(double t) const
{
    double result = 0.0;
    for (unsigned k = 0; k < n; k++)
    {
		double fa = udat[k];
		for (unsigned j = 0; j < n; j++)
		{
			if (j != k)
			{
		        fa *= 1.0/(tdat[k]-tdat[j]);
		    }
		}
		double dl = 0.0;
		for (unsigned j = 0; j < n; j++)
		{
			if (j != k)
			{
				double ll = 1.0;
				for (unsigned i = 0; i < n; i++)
				{
					if (i != j && i != k) ll *= t-tdat[i];
				}
				dl += ll;
			}
		}
		result += fa*dl;
	}
    return result;
}

adevs::InterPoly::~InterPoly()
{
    delete [] tdat;
    delete [] udat;
}
