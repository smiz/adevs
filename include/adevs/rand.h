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
#ifndef __adevs_rand_h_
#define __adevs_rand_h_

#include <cmath>
#include <cstdlib>
#include "adevs/adevs.h"

#define PI 3.14159265358979323846

static double cof[6] = {76.18009173,  -86.50532033,   24.01409822,
                        -1.231739516, 0.120858003e-2, -0.536382e-5};

namespace adevs {

/*
 * The random_seq class is an abstract interface to a random sequence
 * generator.
 */
class random_seq {
  public:
    /// Set the seed for the random number generator
    virtual void set_seed(unsigned long seed) = 0;
    /// Get the next double uniformly distributed in [0, 1]
    virtual double next_dbl() = 0;
    /// Copy the random number generator
    virtual random_seq* copy() const = 0;
    /// Get the next unsigned long
    virtual unsigned long next_long() = 0;
    /// Destructor
    virtual ~random_seq() {}
};

/*
 * The crand class provides random number sequences using the standard
 * C rand_r() function. Each instance of crand generates its own random
 * number sequence, and the clone method saves the state of the random
 * number generator. This class can be used in parallel simulations.
 */
class crand : public random_seq {
  public:
    /// Create a generator with the default seed
    crand() : seedp(0) {}
    /// Copy constructor
    crand(crand const &src) : seedp(src.seedp) {}
    /// Create a generator with the given seed
    crand(unsigned long seed) : seedp((unsigned int)seed) {}
    /// Set the seed for the random number generator
    void set_seed(unsigned long seed) { seedp = (unsigned int)seed; }
    /// Get the next double uniformly distributed in [0, 1]
    double next_dbl() {
#ifdef _WIN32
        return ((double)next_long() / (double)UINT_MAX);
#else
        return ((double)next_long() / (double)RAND_MAX);
#endif
    }
    /// Get the next unsigned long
    // crand random number generator
    unsigned long next_long() {
#ifdef _WIN32
        rand_s(&seedp);
        return seedp;
#else
        return rand_r(&seedp);
#endif
    }
    /// Copy the random number generator
    random_seq* copy() const { return new crand(*this); }
    /// Destructor
    ~crand() {}

  private:
    unsigned int seedp;
};

/*
 * <p>The rv class provides a random variable based on a selectable
 * implementation.  By default, this implementation is crand.
 * I recommend that you find a modern random variate generator,
 * such as the ones included in the new C++ standards,
 * rather than use the one here.</p>
 * <p>The assortment of random variable types was contributed by
 * Alex Cave (who, at the time, was with the Intelligent
 * Systems Automation Group in the School of Engineering at
 * Deakin University).</p>
 */
class rv {
  public:
    /// Create a random variable with the default implementation.
    rv(unsigned long seed) { _impl = new crand(seed); }

    /*
     * Create a random variable with the desired implementation.  The
     * implementation class is adopted by the rv.
     */
    rv(random_seq* impl) { _impl = impl; }

    /// Copy constructor relies on copy method of underlying stream.
    rv(rv const &src) { _impl = src._impl->copy(); }

    /// Assignment operator relies on copy method of underlying stream.
    rv const &operator=(rv const &src) {
        delete _impl;
        _impl = src._impl->copy();
        return *this;
    }

    /// See the random number generator implementation
    void set_seed(unsigned long seed) { _impl->set_seed(seed); }

    /// Get a raw value from the underlying random number generator
    unsigned long next_long() { return (_impl->next_long()); }

    /// Sample a triangular distribution with minimum a, maximum b,
    /// and mode c.
    double triangular(double a, double b, double c) {
        double x = _impl->next_dbl();
        double Fc = (c - a) / (b - a);
        if (x < Fc) {
            return a + sqrt(x * (b - a) * (c - a));
        } else {
            return b - sqrt((1.0 - x) * (b - a) * (b - c));
        }
    }


    /// Sample a uniform distribution in the range [a, b]
    double uniform(double a, double b) {
        return (_impl->next_dbl() * (b - a) + a);
    }

    /*
     * Sample a normally distributed random variable with mean m and
     * standard deviation s.
     */
    double normal(double m, double s) {
        return m + (sqrt(-2.0 * log(_impl->next_dbl())) *
                    cos(PI * (2.0 * _impl->next_dbl() - 1.0))) *
                       s;
    }

    /*
     * return a negative exponentially distributed random number with
     * the mean as parameter
     */
    double exponential(double a) {
        if (a < 0) {
            err(ERREXPONENT);
        }
        return (a * (-log(_impl->next_dbl())));
    }

    /*
     * return a hyperexponentially distributed random number with
     * the means as parameters to two exponentially distributed variates,
     * the first with a chance of p of being generated, the second with a
     * chance of 1-p.
     */
    double hyperexponential(double p, double a, double b) {
        if ((a < 0) || (b < 0)) {
            err(ERREXPONENT);
            return (0);
        } else if ((p <= 0) || (p >= 1)) {
            err(PROBVAL);
            return (0);
        } else if (probability(p)) {
            return (a * (-log(_impl->next_dbl())));
        } else {
            return (b * (-log(_impl->next_dbl())));
        }
    }

    /// Return a laplace number with the given parameter
    double laplace(double a) {
        if (probability(0.5)) {
            return (a * (-log(_impl->next_dbl())));
        } else {
            return (a * (log(_impl->next_dbl())));
        }
    }

    /// Sample a chisquare random variable with n degrees of freedom
    double chisquare(unsigned int n) { return gamma(0.5 * n, 0.5); }

    /// Sample a student-t random variable
    double student(unsigned int n) {
        return (normal(0.0, 1.0) / sqrt(chisquare(n) / n));
    }

    /// Sample a lognormal random variable
    double lognormal(double a, double b) {
        if (b < 0.0) {
            err(ERRLOGNORM);
        }
        return exp(normal(a, b));
    }

    /// Sample an erlang distribution
    double erlang(unsigned int n, double a) {
        if (a < 0) {
            err(ERRERLANG);
            return (0.0);
        } else {
            // use gamma distribution
            return (gamma((double)n, a));
        }
    }

    /// Sample a gamma random variable
    double gamma(double a, double b) {
        // rejection method only for a > 1
        double xx, yy;

        if ((a < 0.0) || (b < 0.0)) {
            err(ERRGAMMA);
        }
        if (a < 1.0) {
            do {
                xx = pow(_impl->next_dbl(), 1.0 / a);
                yy = pow(_impl->next_dbl(), 1.0 / (1.0 - a));
            } while (xx + yy > 1.0);
            xx = xx / (xx + yy);
            yy = exponential(1);
            return xx * yy / b;
        }
        if (a == 1.0) {
            return (exponential(1.0) / b);
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
        do {
            U = 1;
            for (cnt = 0; m > cnt; cnt++) {
                U = U * _impl->next_dbl();
            }
            X = -1 * log(U) * (a / m);
            T = pow((X / a), p) * exp(-1 * p * ((X / a) - 1));
        } while (_impl->next_dbl() > T);
        return (X * b);
    }

    /// Sample a beta random variable
    double beta(double a, double b) {
        if (b == 0.0) {
            err(ERRBETA);
        }
        double zz = gamma(a, 1.0);
        return (zz / (zz + gamma(b, 1.0)));
    }

    /// Sample a F-distributed random variable
    double fdistribution(unsigned int n, unsigned int m) {
        return ((m * chisquare(n)) / (n * chisquare(m)));
    }

    /// Sample a Poisson random variable
    double poisson(double a) {
        // can we use the direct method
        double sq = -1.0, alxm = -1.0, g = -1.0, oldm = -1.0;
        double em, t, yy;
        if (a < 12.0) {
            if (a != oldm) {
                oldm = a;
                g = exp(-a);
            }
            em = -1.0;
            t = 1.0;
            do {
                em += 1.0;
                t *= _impl->next_dbl();
            } while (t > g);
        }
        // use the rejection method
        else {
            if (a != oldm) {
                oldm = a;
                sq = sqrt(2.0 * a);
                alxm = log(a);
                g = a * alxm - lngamma(a + 1.0);
            }
            do {
                do {
                    // y is a deviate from a Lorentzian comparison function
                    yy = tan(PI * _impl->next_dbl());
                    em = sq * yy + a;
                } while (em < 0.0);
                em = floor(em);
                t = 0.9 * (1.0 + yy * yy) *
                    exp(em * alxm - lngamma(em + 1.0) - g);
            } while (_impl->next_dbl() > t);
        }
        return em;
    }

    /// Sample a geometric random variable with event probability p
    double geometric(double p) {
        if ((p <= 0.0) || (p >= 1.0)) {
            err(PROBVAL);
        }
        return (ceil(log(_impl->next_dbl()) / log(1.0 - p)));
    }

    /*
     * return a variate from the hypergeometric distribution with m the
     * population, p the chance on success and n the number of items drawn
     */
    double hypergeometric(unsigned int m, unsigned int n, double p) {
        if ((p <= 0.0) || (p >= 1.0)) {
            err(PROBVAL);
        } else if (m < n) {
            err(ERRHYPGEO);
        }
        double g = p * m;  // the success items
        double c = m - g;  // the non-success items
        int k = 0;
        for (unsigned int i = 0; i < n; i++)  // n drawings
        {
            if (_impl->next_dbl() <= g / m)  // if a success
            {
                k++;  // success
                g--;  // decrease right items
            } else {
                c--;  // decrease wrong items
            }
            m--;  // decrease total
        }
        return k;  // return deviate
    }

    /// Sample a weibull random variable
    double weibull(double a, double b) {
        return pow(exponential(a), (1.0 / b));
    }

    /*
     *  An event count for a binomial distribution with event
     *  probability p and n the number of trials
     */
    double binomial(double p, unsigned int n) {
        unsigned int j;
        int nold = -1;
        double am, em, g, angle, prob, bnl, sq, t, yy;
        double pold = -1.0;
        double pc = 0, plog = 0, pclog = 0, en = 0, oldg = 0;

        if ((p <= 0.0) || (p >= 1.0)) {
            err(PROBVAL);
        }
        if (p <= 0.5) {
            prob = p;
        } else {
            prob = 1.0 - p;
        }
        am = n * prob;

        if (n < 25) {
            bnl = 0.0;
            for (j = 1; j <= n; j++) {
                if (_impl->next_dbl() < prob) {
                    bnl += 1.0;
                }
            }
        } else if (am < 10) {
            g = exp(-am);
            t = 1.0;
            for (j = 0; j <= n; j++) {
                t = t * _impl->next_dbl();
                if (t < g) {
                    break;
                }
            }
            if (j <= n) {
                bnl = j;
            } else {
                bnl = n;
            }
        } else {
            if ((int)n != nold) {
                en = n;
                oldg = lngamma(en + 1.0);
                nold = n;
            }
            if (prob != pold) {
                pc = 1.0 - prob;
                plog = log(prob);
                pclog = log(pc);
                pold = prob;
            }
            sq = sqrt(2.0 * am * pc);
            do {
                do {
                    angle = PI * _impl->next_dbl();
                    yy = tan(angle);
                    em = sq * yy + am;
                } while (em < 0.0 || em >= (en + 1.0));
                em = floor(em);
                t = 1.2 * sq * (1.0 + yy * yy) *
                    exp(oldg - lngamma(em + 1.0) - lngamma(en - em + 1.0) +
                        em * plog + (en - em) * pclog);
            } while (_impl->next_dbl() > t);
            bnl = em;
        }
        if (prob != p) {
            bnl = n - bnl;
        }
        return bnl;
    }

    /*
     * return a random variable with probabilty of success equal to p
     * and n as the number of successes
     */
    double negativebinomial(double p, unsigned int n) {
        if ((p <= 0.0) || (p >= 1.0)) {
            err(PROBVAL);
        }
        return (poisson(gamma(n, p / (1.0 - p))));
    }

    /// Sample a triangular random variable
    double triangular(double a) {
        if ((a < 0.0) || (a > 1.0)) {
            err(ERRTRIANG);
        }
        double xx = _impl->next_dbl();
        double yy = _impl->next_dbl();
        if (xx > a) {
            return (1.0 - ((1.0 - a) * sqrt(yy)));
        } else {
            return (sqrt(yy) * a);
        }
    }

    /// return TRUE with probability p and FALSE with probability 1-p
    bool probability(double p) {
        if ((p < 0.0) || (p > 1.0)) {
            err(PROBVAL);
        }
        return (p >= _impl->next_dbl());
    }

    /// Sample a lognormal gamma random variable
    double lngamma(double xx) {
        double x = xx - 1.0;
        double tmp = x + 5.5;
        tmp -= (x + 0.5) * log(tmp);
        double ser = 1.0;
        for (int j = 0; j < 6; j++) {
            x += 1.0;
            ser += cof[j] / x;
        }
        return (-tmp + log(2.50662827465 * ser));
    }

    /// Destructor
    ~rv() { delete _impl; }


  private:
    random_seq* _impl;
    typedef enum {
        NDURATION,
        NCOUNT,
        EMPTY,
        NDELAY,
        ADDTOQ,
        EMPTYQ,
        HNCLMS,
        HMODE,
        PROBVAL,
        ERRUNIFORM,
        ERRNORMAL,
        ERRLOGNORM,
        ERRTRIANG,
        ERRGAMMA,
        ERRBETA,
        ERREXPONENT,
        ERRERLANG,
        ERRHYPGEO,
        NULLEV,
        NOHISTO,
        INITERR,
        AMODE,
        HFORM,
        ERRFILE,
        SAMPLE,
        FRACTION,
        LEVEL,
        SCAN,
        SUPPRESS,
        SEED
    } errorType;

    void err(errorType n) {
        static char const* errmsg[] = {
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
            "seeds for the generator out of range"};
        throw adevs::exception(errmsg[n]);
    }
};

}  // namespace adevs

#endif
