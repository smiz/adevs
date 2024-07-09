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
#include <cstdlib>
#include "adevs/adevs.h"

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
    unsigned long next_long();
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
    rv(unsigned long seed = 1);
    /*
     * Create a random variable with the desired implementation.  The
     * implementation class is adopted by the rv.
     */
    rv(random_seq* rand);
    /// Copy constructor relies on copy method of underlying stream.
    rv(rv const &src);
    /// Assignment operator relies on copy method of underlying stream.
    rv const &operator=(rv const &src);
    /// See the random number generator implementation
    void set_seed(unsigned long seed);
    /// Get a raw value from the underlying random number generator
    unsigned long next_long();
    /// Sample a triangular distribution with minimum a, maximum b,
    /// and mode c.
    double triangular(double a, double b, double c);
    /// Sample a uniform distribution in the range [a, b]
    double uniform(double a, double b);
    /*
     * Sample a normally distributed random variable with mean m and
     * standard deviation s.
     */
    double normal(double m, double s);
    /*
     * return a negative exponentially distributed random number with
     * the mean as parameter
     */
    double exponential(double a);
    /*
     * return a hyperexponentially distributed random number with
     * the means as parameters to two exponentially distributed variates,
     * the first with a chance of p of being generated, the second with a
     * chance of 1-p.
     */
    double hyperexponential(double p, double a, double b);
    /// Return a laplace number with the given parameter
    double laplace(double a);
    /// Sample a chisquare random variable with n degrees of freedom
    double chisquare(unsigned int n);
    /// Sample a student-t random variable
    double student(unsigned int n);
    /// Sample a lognormal random variable
    double lognormal(double a, double b);
    /// Sample an erlang distribution
    double erlang(unsigned int n, double a);
    /// Sample a gamma random variable
    double gamma(double a, double b);
    /// Sample a beta random variable
    double beta(double a, double b);
    /// Sample a F-distributed random variable
    double fdistribution(unsigned int n, unsigned int m);
    /// Sample a Poisson random variable
    double poisson(double a);
    /// Sample a geometric random variable with event probability p
    double geometric(double p);
    /*
     * return a variate from the hypergeometric distribution with m the
     * population, p the chance on success and n the number of items drawn
     */
    double hypergeometric(unsigned int m, unsigned int n, double p);
    /// Sample a weibull random variable
    double weibull(double a, double b);
    /*
     *  An event count for a binomial distribution with event
     *  probability p and n the number of trials
     */
    double binomial(double p, unsigned int n);
    /*
     * return a random variable with probabilty of success equal to p
     * and n as the number of successes
     */
    double negativebinomial(double p, unsigned int n);
    /// Sample a triangular random variable
    double triangular(double a);
    /// return TRUE with probability p and FALSE with probability 1-p
    bool probability(double p);
    /// Sample a lognormal gamma random variable
    double lngamma(double xx);
    /// Destructor
    ~rv();

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

    void err(errorType n);
};

}  // namespace adevs

#endif
