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
