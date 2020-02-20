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
#define _CRT_RAND_S
#include <cstdlib>
#include "adevs_rand.h"
#include <cmath>
using namespace adevs;

#ifndef PI
#define PI   3.14159265358979323846
#endif

static double cof[6] = 
{
	76.18009173, 
	-86.50532033, 
	24.01409822, 
	-1.231739516,
	0.120858003e-2, 
	-0.536382e-5
};

// crand random number generator
unsigned long crand::next_long() 
{
	#ifdef _WIN32
	rand_s(&seedp);
	return seedp;
	#else
	return rand_r(&seedp);
	#endif
}

// constructor, taking the seeds of the stream
adevs::rv::rv(unsigned long seed) 
{
	_impl = new crand(seed);
}

adevs::rv::rv(random_seq* impl)
{
	_impl = impl;
}

adevs::rv::rv(const rv& src)
{
	_impl = src._impl->copy();
}

const adevs::rv& rv::operator=(const rv& src)
{
	delete _impl;
	_impl = src._impl->copy();
	return *this;
}

void adevs::rv::set_seed(unsigned long seed)
{
	_impl->set_seed (seed);
}

double adevs::rv::triangular(double a, double b, double c)
{
	double x = _impl->next_dbl();
	double Fc = (c-a)/(b-a);
	if (x < Fc) return a+sqrt(x*(b-a)*(c-a));
	else return b-sqrt((1.0-x)*(b-a)*(b-c));
}


double adevs::rv::uniform(double a, double b) 
{
	return (_impl->next_dbl() * (b - a) + a);
}


double adevs::rv::normal(double m, double s) 
{
	return m + (sqrt (-2.0 * log (_impl->next_dbl())) * 
		cos (PI * (2.0 * _impl->next_dbl() - 1.0))) * s;
}


adevs::rv::~rv() 
{ 
	delete _impl; 
}


double adevs::rv::exponential(double a)
{
	if (a < 0)
	{
		err(ERREXPONENT);
 	} 
	return (a * (-log(_impl->next_dbl())));
}


double adevs::rv::hyperexponential(double p,double a,double b)
{
	if ( (a < 0) || (b < 0) )
	{
		err(ERREXPONENT);
		return(0);
	}
	else if ( (p <= 0) || (p >= 1) )
	{
		err(PROBVAL);
		return(0);
	}
	else if (probability(p))
	{
		return (a * (-log(_impl->next_dbl())));
	}
	else
	{
		return (b * (-log(_impl->next_dbl())));
	}
}

double adevs::rv::laplace(double a)
{
	if (probability(0.5))
	{
		return (a * ( -log(_impl->next_dbl()) ));
	}
	else
	{
		return (a * ( log(_impl->next_dbl()) ));
	}
}

double adevs::rv::erlang(unsigned int n,double a)
{
	if (a < 0)
	{
		err(ERRERLANG);
		return(0.0);
	}
	else 
	{
		// use gamma distribution
		return (gamma((double)n,a));
	}
}

// return random number from the gamma distribution
double adevs::rv::gamma(double a,double b)
{
	// rejection method only for a > 1
	double xx,yy;

	if ((a < 0.0) || (b < 0.0))
	{
		err(ERRGAMMA);
	}
	if (a < 1.0) 
	{
		do
		{
			xx = pow(_impl->next_dbl(),1.0/a);
			yy = pow(_impl->next_dbl(),1.0/(1.0-a));
		}
		while (xx+yy > 1.0);
		xx = xx/(xx+yy);
		yy = exponential(1);
		return xx*yy/b;
	}
	if (a == 1.0)
	{
		return (exponential (1.0) / b);
	}

	/*
	For this method I refer reader to
	Computer Generation of Gamma Random Variables
	Tadikamalla, P,R. 
	Management Science / Operations Research
	Vol 21 Iss 5, May 1978
	*/

	int cnt;
	int m = (int)floor(a);
	double U, X, T;
	double p = a - m;
	do
	{
		U=1;
		for( cnt = 0; m > cnt; cnt++)
		{
			U = U*_impl->next_dbl();
		}
		X = -1*log(U)*(a/m);
		T = pow((X/a),p)*exp(-1*p*((X/a)-1));
	} 
	while( _impl->next_dbl() > T );
	return(X*b);
}


// return a random variable from a chi square distribution with
// n degrees of freedom
double adevs::rv::chisquare(unsigned int n)
{
	return gamma(0.5*n,0.5);
}

// return random variable from a beta distribution
double adevs::rv::beta(double a,double b)
{
	if (b == 0.0)
	{
		err(ERRBETA);
	}
	double zz = gamma(a,1.0);
	return (zz/(zz+gamma(b,1.0)));
}

// return it from the student distribution
double adevs::rv::student(unsigned int n)
{
	return (normal(0.0,1.0)/sqrt(chisquare(n)/n));
} 

// return a random number from a lognormal distribution
double adevs::rv::lognormal(double a,double b)
{
	if (b < 0.0) 
	{
		err(ERRLOGNORM);
	}
	return exp(normal(a,b));
} 

// return it from a f distribution.
double adevs::rv::fdistribution(unsigned int n,unsigned int m)
{
	return ( (m * chisquare(n)) / (n * chisquare(m)) );
}

// random number from a weibull distribution
double adevs::rv::weibull (double a,double b)
{
	return pow(exponential(a),(1.0/b));
} 


// get it from a poisson distribution with the given mean
double adevs::rv::poisson(double a)
{
	// can we use the direct method
	double sq=-1.0,alxm=-1.0,g=-1.0,oldm = -1.0;
	double em,t,yy;
	if (a < 12.0) 
	{
		if (a != oldm) 
		{
			oldm = a;
			g = exp(-a);
		}
		em = -1.0;
		t = 1.0;
		do 
		{
			em += 1.0;
			t *= _impl->next_dbl();
		} 
		while (t > g);
	}
	// use the rejection method
	else 
	{
		if (a != oldm) 
		{
			oldm = a;
			sq = sqrt(2.0 * a);
			alxm = log(a);
			g = a*alxm-lngamma(a + 1.0);
		}
		do 
		{
			do 
			{
				// y is a deviate from a Lorentzian comparison function
				yy = tan (PI * _impl->next_dbl());
				em = sq * yy + a;
			} 
			while (em < 0.0);
			em = floor(em);
			t = 0.9 * (1.0 + yy * yy) * exp(em * alxm - lngamma(em + 1.0) - g);
		} 
		while (_impl->next_dbl() > t);
	}
	return em;
} 

// return a variate from the geometric distribution with event
// probability p
double adevs::rv::geometric(double p)
{
	if ((p <= 0.0) || (p >= 1.0))
	{
		err(PROBVAL);
	}
	return (ceil(log(_impl->next_dbl())/log(1.0 - p)));
}

// return a variate from the hypergeometric distribution with m the
// population, p the chance on success and n the number of items drawn
double adevs::rv::hypergeometric(unsigned int m,unsigned int n,double p)
{
	if ((p <= 0.0) || (p >= 1.0))
	{
		err(PROBVAL);
	}
	else if (m < n)
	{
		err(ERRHYPGEO);
	}
	double g = p*m;              // the success items
	double c = m-g;              // the non-success items
	int k=0;
	for (unsigned int i=0;i<n;i++)        // n drawings
	{
		if (_impl->next_dbl() <= g/m)       // if a success
		{
			k++;                     // success
			g--;                     // decrease right items
		}
		else
		{
			c--;                     // decrease wrong items
		}
		m--;                       // decrease total
	}
	return k;                    // return deviate
}

// get it from the binomial distribution with event probability p
// and n the number of trials
double adevs::rv::binomial(double p,unsigned int n)
{
	unsigned int j;
	int nold = -1;
	double am,em,g,angle,prob,bnl,sq,t,yy;
	double pold = -1.0;
	double pc=0,plog=0,pclog=0,en=0,oldg=0;

	if ((p <= 0.0) || (p >= 1.0))
	{
		err(PROBVAL);
	}
	if (p <= 0.5 )
	{
		prob = p ;
	}
	else
	{
		prob = 1.0 - p ;
	}
	am = n * prob;

	if (n < 25)
	{
		bnl = 0.0;
		for (j = 1;j <= n;j++)
		{
			if (_impl->next_dbl() < prob)
			{
				bnl += 1.0;
			}
		}
	}
	else if (am < 10)
	 {
		g = exp(-am);
		t = 1.0;
		for (j = 0;j <= n;j++) 
		{
			t = t * _impl->next_dbl();
			if (t < g) break;
		}
		if (j <= n)
		{
			bnl = j;
		}
		else
		{
			bnl = n;
		}
	}
	else 
	{
		if ((int)n != nold) 
		{
			en = n;
			oldg = lngamma(en + 1.0);
			nold = n;
		}
		if (prob != pold) 
		{
			pc = 1.0 - prob;
			plog = log(prob);
			pclog = log(pc);
			pold = prob;
		}
		sq = sqrt( 2.0 * am * pc );
		do 
		{
			do 
			{
				angle = PI * _impl->next_dbl();
				yy = tan(angle);
				em = sq * yy + am;
			} 
			while (em < 0.0 || em >= (en + 1.0));
			em = floor(em);
			t = 1.2 * sq * (1.0 + yy * yy) * exp(oldg - lngamma(em + 1.0)
				- lngamma(en - em + 1.0) + em * plog + (en - em) * pclog);
		} 
		while (_impl->next_dbl() > t);
		bnl = em;
	}
	if (prob != p)
	{
		bnl = n - bnl;
	}
	return bnl;
}

// return a random variable with probabilty of success equal to p
// and n as the number of successes
double adevs::rv::negativebinomial(double p,unsigned int n)
{
	if ((p <= 0.0) || (p >= 1.0))
	{
		err(PROBVAL);
	}
	return (poisson(gamma(n,p/(1.0-p))));
} 

double adevs::rv::lngamma(double xx)
{
	double x = xx - 1.0;
	double tmp = x + 5.5;
	tmp -= (x + 0.5) * log(tmp);
	double ser = 1.0;
	for (int j = 0;j < 6;j++) 
	{
		x += 1.0;
		ser += cof[j] / x;
	}
	return (-tmp + log(2.50662827465 * ser));
} 

// return a random variable drawn from the triangular distribution
double adevs::rv::triangular(double a)
{
	if ((a < 0.0) || (a > 1.0)) 
	{
		err(ERRTRIANG);
	}
	double xx = _impl->next_dbl();
	double yy = _impl->next_dbl();
	if (xx > a) 
	{
		return ( 1.0 - (( 1.0 - a) * sqrt(yy)) );
	}
	else
	{
		return (sqrt(yy) * a);
	}
} 


// return OK with probability p and FALSE with probability 1-p
bool adevs::rv::probability(double p)
{
	if ((p < 0.0) || (p > 1.0)) 
	{
		err(PROBVAL);
	}
	return (p >= _impl->next_dbl());
}

void adevs::rv::err(errorType n)
{
	static const char* errmsg[] =
	{
		"negative simulation duration",
		"negative count",
		"scheduled events list empty",
		"cannot have negative time delay",
		"cannot add NULL event to the queue",
		"cannot remove an event from an empty queue",
		"illegal number of columns in histogram",
		"illegal histogram mode",
		"probability values must be in range 0 to 1",
		"must have upper limit > lower limit in uniform distribution",
		"negative standard deviation in normal distribution",
		"negative standard deviation in log-normal distribution", 
		"mode out of range in triangular distribution",
		"value of arguments less than zero",
		"argument equal to zero in beta function",
		"negative mean for exponential",
		"negative mean for erlang",
		"population size less then number drawn in hypergeometric",
		"NULL event",
		"NULL histogram",
		"simulation object not created or created after other objects",
		"illegal analysis mode",
		"illegal histogram form",
		"cannot open the file",
		"cannot find a file",
		"fraction world / screen differs between width and height",
		"wrong confidence level",
		"wrong initialization of scan flag",
		"wrong initialization of suppress flag",
		"seeds for the generator out of range"
	};
	throw adevs::exception(errmsg[n]);
}

unsigned long adevs::rv::next_long()
{
	return( _impl->next_long() );
}
