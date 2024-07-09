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
#ifndef __adevs_time_h_
#define __adevs_time_h_
#include <cfloat>
#include <cmath>
#include <iostream>
#include <limits>

/// Returns the maximum value for a time type
template <class T>
inline T adevs_inf();
/// Returns the zero value for a time type
template <class T>
inline T adevs_zero();
/// Returns a value less than adevs_zero()
template <class T>
inline T adevs_sentinel();
/// Returns the interval to the next instant of time
template <class T>
inline T adevs_epsilon();

namespace adevs {

/*
 * This time type allows models to evolve on R x Z.
 */
template <typename T = double>
class sd_time {
  public:
    /// Creates the identify (0,0)
    sd_time() : t(0), k(0) {}
    /// Create a time (t,k)
    sd_time(T t, int k) : t(t), k(k) {}
    /// Copy constructor
    sd_time(sd_time const &other) : t(other.t), k(other.k) {}
    /// Get the real part of time
    T real() const { return t; }
    /// Get the logical part of time
    double integer() const { return k; }
    /// Assignment operator
    sd_time const &operator=(sd_time const &other) {
        t = other.t;
        k = other.k;
        return *this;
    }
    /// Equivalence
    bool operator==(sd_time const &t2) const {
        return (t == t2.t && k == t2.k);
    }
    /// Not equal
    bool operator!=(sd_time const &t2) const { return !(*this == t2); }
    /// Order by t then by c
    bool operator<(sd_time const &t2) const {
        return (t < t2.t || (t == t2.t && k < t2.k));
    }
    /// Less than or equal
    bool operator<=(sd_time const &t2) const {
        return (*this == t2 || *this < t2);
    }
    /// Greater than
    bool operator>(sd_time const &t2) const { return !(*this <= t2); }
    /// Greater than or equal
    bool operator>=(sd_time const &t2) const { return !(*this < t2); }
    /// Advance this value by a step size t2
    sd_time operator+(sd_time const &t2) const {
        sd_time result(*this);
        result += t2;
        return result;
    }
    /// Advance this value by a step size t2
    sd_time const &operator+=(sd_time const &t2) {
        if (t2.t == 0) {
            k += t2.k;
        } else {
            t += t2.t;
            k = t2.k;
        }
        return *this;
    }
    /// Length of the interval from now to t2
    sd_time operator-(sd_time const &t2) const {
        sd_time result(*this);
        result -= t2;
        return result;
    }
    /// Length of the interval from now to t2
    sd_time const &operator-=(sd_time const &t2) {
        if (t == t2.t) {
            t = 0;
            k -= t2.k;
        } else {
            t -= t2.t;
        }
        return *this;
    }
    /// Print a time to the output stream
    friend std::ostream &operator<<(std::ostream &out, sd_time const &t) {
        out << "(" << t.t << "," << t.k << ")";
        return out;
    }
    /// Read a time from the input stream
    friend std::istream &operator>>(std::istream &in, sd_time &t) {
        char junk;
        in >> junk >> t.t >> junk >> t.k >> junk;
        return in;
    }

  private:
    T t;
    int k;
};

/*
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
inline int fcmp(double x1, double x2, double epsilon) {
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

    if (difference > delta) {
        return 1; /* x1 > x2 */
    } else if (difference < -delta) {
        return -1; /* x1 < x2 */
    } else {       /* -delta <= difference <= delta */
        return 0;  /* x1 == x2 */
    }
}

/*
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
    /*
     * The user must instantiate this static variable
     * and initialize as required by the fcmp function.
     */
    static double epsilon;

    double_fcmp(double rhs = 0) : d(rhs) {}

    double_fcmp const &operator=(double_fcmp const &rhs) {
        d = rhs.d;
        return *this;
    }
    double_fcmp const &operator=(double rhs) {
        d = rhs;
        return *this;
    }
    operator double() { return d; }
    bool operator<(double rhs) const { return (fcmp(d, rhs, epsilon) < 0); }
    bool operator<(double_fcmp const &rhs) const {
        return (fcmp(d, rhs.d, epsilon) < 0);
    }
    bool operator<=(double_fcmp const &rhs) const {
        return (fcmp(d, rhs.d, epsilon) <= 0);
    }
    bool operator>(double_fcmp const &rhs) const {
        return (fcmp(d, rhs.d, epsilon) > 0);
    }
    bool operator>=(double_fcmp const &rhs) const {
        return (fcmp(d, rhs.d, epsilon) >= 0);
    }
    bool operator==(double rhs) const { return (fcmp(d, rhs, epsilon) == 0); }
    bool operator==(double_fcmp const &rhs) const {
        return (fcmp(d, rhs.d, epsilon) == 0);
    }
};

}  // namespace adevs

template <>
inline float adevs_inf() {
    return std::numeric_limits<float>::max();
}
template <>
inline long double adevs_inf() {
    return std::numeric_limits<long double>::max();
}
template <>
inline double adevs_inf() {
    return std::numeric_limits<double>::max();
}
template <>
inline int adevs_inf() {
    return std::numeric_limits<int>::max();
}
template <>
inline long adevs_inf() {
    return std::numeric_limits<long>::max();
}
template <>
inline adevs::double_fcmp adevs_inf() {
    return std::numeric_limits<double>::max();
}
template <>
inline adevs::sd_time<double> adevs_inf() {
    return adevs::sd_time<double>(std::numeric_limits<double>::max(),
                                  std::numeric_limits<int>::max());
}
template <>
inline adevs::sd_time<long> adevs_inf() {
    return adevs::sd_time<long>(std::numeric_limits<long>::max(),
                                std::numeric_limits<int>::max());
}
template <>
inline adevs::sd_time<int> adevs_inf() {
    return adevs::sd_time<int>(std::numeric_limits<int>::max(),
                               std::numeric_limits<int>::max());
}

template <>
inline float adevs_zero() {
    return 0.0f;
}
template <>
inline long double adevs_zero() {
    return 0.0L;
}
template <>
inline double adevs_zero() {
    return 0.0;
}
template <>
inline int adevs_zero() {
    return 0;
}
template <>
inline long adevs_zero() {
    return 0;
}
template <>
inline adevs::double_fcmp adevs_zero() {
    return 0.0;
}
template <>
inline adevs::sd_time<double> adevs_zero() {
    return adevs::sd_time<double>(0.0, 0);
}
template <>
inline adevs::sd_time<long> adevs_zero() {
    return adevs::sd_time<long>(0, 0);
}
template <>
inline adevs::sd_time<int> adevs_zero() {
    return adevs::sd_time<int>(0, 0);
}

template <>
inline float adevs_sentinel() {
    return -1.0f;
}
template <>
inline long double adevs_sentinel() {
    return -1.0L;
}
template <>
inline double adevs_sentinel() {
    return -1.0;
}
template <>
inline int adevs_sentinel() {
    return -1;
}
template <>
inline long adevs_sentinel() {
    return -1;
}
template <>
inline adevs::double_fcmp adevs_sentinel() {
    return -1.0;
}
template <>
inline adevs::sd_time<double> adevs_sentinel() {
    return adevs::sd_time<double>(-1.0, 0);
}
template <>
inline adevs::sd_time<long> adevs_sentinel() {
    return adevs::sd_time<long>(-1, 0);
}
template <>
inline adevs::sd_time<int> adevs_sentinel() {
    return adevs::sd_time<int>(-1, 0);
}

template <>
inline float adevs_epsilon() {
    return 0.0f;
}
template <>
inline long double adevs_epsilon() {
    return 0.0L;
}
template <>
inline double adevs_epsilon() {
    return 0.0;
}
template <>
inline int adevs_epsilon() {
    return 0;
}
template <>
inline long adevs_epsilon() {
    return 0;
}
template <>
inline adevs::double_fcmp adevs_epsilon() {
    return 0.0;
}
template <>
inline adevs::sd_time<double> adevs_epsilon() {
    return adevs::sd_time<double>(0.0, 1);
}
template <>
inline adevs::sd_time<long> adevs_epsilon() {
    return adevs::sd_time<long>(0, 1);
}
template <>
inline adevs::sd_time<int> adevs_epsilon() {
    return adevs::sd_time<int>(0, 1);
}

#endif
