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
#ifndef __adevs_time_h_
#define __adevs_time_h_
#include <cfloat>
#include <iostream>
#include <cmath>
#include <limits>

/// Returns the maximum value for a time type
template <class T> inline T adevs_inf();
/// Returns the zero value for a time type
template <class T> inline T adevs_zero(); 
/// Returns a value less than adevs_zero()
template <class T> inline T adevs_sentinel(); 

namespace adevs
{

/**
 * This is the super dense simulation clock that is used by
 * the parallel simulator to properly manage simultaneous
 * events.
 */
template<class T = double> struct Time
{
	T t;
	unsigned int c;
	/// Value for infinity
	static adevs::Time<T> Inf() { return Time<T>(adevs_inf<T>(),0); }
	/// Constructor. Default time is (0,0).
	Time(T t = adevs_zero<T>(), unsigned int c = 0):t(t),c(c){}
	/// Copy constructor
	Time(const Time& t2):t(t2.t),c(t2.c){}
	/// Assignment operator
	const Time& operator=(const Time& t2)
	{
		t = t2.t;
		c = t2.c;
		return *this;
	}
	/// Comparing with a T compares the real field
	bool operator<(T t2) const { return t < t2; }
	/**
	 * Assigning a T sets the real field to the T and
	 * the integer field to zero
	 */
	const Time& operator=(T t2)
	{
		t = t2;
		c = 0;
		return *this;
	}
	/// Advance operator (this is not commutative or associative!)
	Time operator+(const Time& t2) const
	{
	   if (t2.t == 0) return Time(t,t2.c+c);
	   else return Time(t+t2.t,0);
	}
	/// Advance and assign
	const Time& operator+=(const Time& t2)
	{
		*this = *this+t2;
		return *this;
	}
	/// Subtract a real number (used to get the elapsed time)
	T operator-(T t2) const
	{
		return t-t2;
	}
	/// Equivalence
	bool operator==(const Time& t2) const
	{
		return (t == t2.t && c == t2.c);
	}
	/// Not equal
	bool operator!=(const Time& t2) const
	{
		return !(*this == t2);
	}
	/// Order by t then by c
	bool operator<(const Time& t2) const
	{
		return (t < t2.t || (t == t2.t && c < t2.c));
	}
	bool operator<=(const Time& t2) const
	{
		return (*this == t2 || *this < t2);
	}
	bool operator>(const Time& t2) const
	{
		return !(*this <= t2);
	}
	bool operator>=(const Time& t2) const
	{
		return !(*this < t2);
	}
};

/**
 * <p>The fcmp() inline function is taken from fcmp.c, which is
 * Copyright (c) 1998-2000 Theodore C. Belding,
 * University of Michigan Center for the Study of Complex Systems,
 * <mailto:Ted.Belding@umich.edu>,
 * <http://www-personal.umich.edu/~streak/>,
 * </p>
 *
 * <p>This code is part of the fcmp distribution. fcmp is free software;
 * you can redistribute and modify it under the terms of the GNU Library
 * General Public License (LGPL), version 2 or later.  This software
 * comes with absolutely no warranty.</p>
 */ 
inline int fcmp(double x1, double x2, double epsilon) 
{
    int exponent;
    double delta;
    double difference;

    /* Get exponent(max(fabs(x1), fabs(x2))) and store it in exponent. */

    /* If neither x1 nor x2 is 0, */
    /* this is equivalent to max(exponent(x1), exponent(x2)). */

    /* If either x1 or x2 is 0, its exponent returned by frexp would be 0, */
    /* which is much larger than the exponents of numbers close to 0 in */
    /* magnitude. But the exponent of 0 should be less than any number */
    /* whose magnitude is greater than 0. */

    /* So we only want to set exponent to 0 if both x1 and */
    /* x2 are 0. Hence, the following works for all x1 and x2. */

    frexp(fabs(x1) > fabs(x2) ? x1 : x2, &exponent);

    /* Do the comparison. */

    /* delta = epsilon * pow(2, exponent) */

    /* Form a neighborhood around x2 of size delta in either direction. */
    /* If x1 is within this delta neighborhood of x2, x1 == x2. */
    /* Otherwise x1 > x2 or x1 < x2, depending on which side of */
    /* the neighborhood x1 is on. */

    delta = ldexp(epsilon, exponent); 

    difference = x1 - x2;

    if (difference > delta)
        return 1; /* x1 > x2 */
    else if (difference < -delta)
        return -1;  /* x1 < x2 */
    else /* -delta <= difference <= delta */
        return 0;  /* x1 == x2 */
}

/**
 * This is an alternative double that may be used for the simulation clock
 * (i.e., as the template parameter T for models and simulators). It
 * uses the fcmp function to check for equality instead of the 
 * default equality operator. Information on the fcmp function
 * may be found at http://fcmp.sourceforge.net/
 */
class double_fcmp {

private:
	double d;

public:
    /**
	 * The user must instantiate this static variable
	 * and initialize as required by the fcmp function.
	 */
    static double epsilon;

    double_fcmp(double rhs = 0) 
        : d(rhs) { }

    const double_fcmp& operator=(const double_fcmp& rhs)
    {
        d = rhs.d;
        return *this;
    }
    const double_fcmp& operator=(double rhs)
    {
        d = rhs;
        return *this;
    }
    operator double()
    {
        return d;
    }
    bool operator<(double rhs) const
    {
        return (fcmp(d, rhs, epsilon) < 0);
    }
    bool operator<(const double_fcmp& rhs) const
    {
        return (fcmp(d, rhs.d, epsilon) < 0);
    }
    bool operator<=(const double_fcmp& rhs) const
    {
        return (fcmp(d, rhs.d, epsilon) <= 0);
    }
    bool operator>(const double_fcmp& rhs) const
    {
        return (fcmp(d, rhs.d, epsilon) > 0);
    }
    bool operator>=(const double_fcmp& rhs) const
    {
        return (fcmp(d, rhs.d, epsilon) >= 0);
    }
    bool operator==(double rhs) const
    {
        return (fcmp(d, rhs, epsilon) == 0);
    }
    bool operator==(const double_fcmp& rhs) const
    {
        return (fcmp(d, rhs.d, epsilon) == 0);
    }
};

} // end namespace

template <> inline double adevs_inf() {
	return std::numeric_limits<double>::max(); }
template <> inline int adevs_inf() {
	return std::numeric_limits<int>::max(); }
template <> inline long adevs_inf() {
	return std::numeric_limits<long>::max(); }
template <> inline adevs::double_fcmp adevs_inf() {
	return std::numeric_limits<double>::max(); }

template <> inline double adevs_zero() { return 0.0; }
template <> inline int adevs_zero() { return 0; }
template <> inline long adevs_zero() { return 0; }
template <> inline adevs::double_fcmp adevs_zero() { return 0.0; }

template <> inline double adevs_sentinel() { return -1.0; }
template <> inline int adevs_sentinel() { return -1; }
template <> inline long adevs_sentinel() { return -1; }
template <> inline adevs::double_fcmp adevs_sentinel() { return -1.0; }

template<class T>
std::ostream& operator<<(std::ostream& strm, const adevs::Time<T>& t);

#endif
