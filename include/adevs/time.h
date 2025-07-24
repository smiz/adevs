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

#ifndef _adevs_time_h_
#define _adevs_time_h_

#include <cfloat>
#include <cmath>
#include <iostream>
#include <limits>


/// @brief Returns the maximum value for a time type
template <class TimeType>
inline TimeType adevs_inf();
/// @brief Returns the zero value for a time type
template <class TimeType>
inline TimeType adevs_zero();
/// @brief Returns a value less than adevs_zero()
template <class TimeType>
inline TimeType adevs_sentinel();
/**
 * @brief Returns the smallest possible interval of time.
 * 
 * If time is real (the default) or integers then this is zero.
 * It takes on non-zero values for super dense time types.
 */
template <class TimeType>
inline TimeType adevs_epsilon();


namespace adevs {

/**
 * @brief A super dense time type.
 * 
 * This time type is a pair R x Z. The first element is a real number
 * representing in whatever are the units of your model. The second element
 * is an integer that distinguishes between different events that occur at the
 * same time. The order is lexicographic, so that (t1,k1) < (t2,k2) if t1 < t2
 * or (t1 == t2 and k1 < k2). Addition when t1 != t2 sets the integer part to
 * zero. If t1 = t2 then the the integer parts are added together. The length
 * of an interval is equal to the difference of the real parts if they are
 * different, or the difference of the integer parts if the real parts are the same.
 * 
 * You can find a description of a commonly used for of super dense time in the
 * <a href="https://fmi-standard.org">FMI standard</a>. A more formal treatment
 * is available in the paper <a href="https://dl.acm.org/doi/abs/10.1145/3379489">
 * "Towards a theory of super dense time in simulation models."</a>
 */
template <typename TimeType = double>
class sd_time {
  public:
    /// @brief Creates the zero value (0,0)
    sd_time() : t(0), k(0) {}
    /// @brief Create a time (t,k)
    sd_time(TimeType t, int k) : t(t), k(k) {}
    /// @brief Copy constructor
    sd_time(sd_time const &other) : t(other.t), k(other.k) {}
    /// @brief Get the real part of time
    TimeType real() const { return t; }
    /// @brief Get the logical (integer) part of time
    double integer() const { return k; }
    /// @brief Assignment operator
    sd_time const &operator=(sd_time const &other) {
        t = other.t;
        k = other.k;
        return *this;
    }
    /// @brief Equivalence
    bool operator==(sd_time const &t2) const {
        return (t == t2.t && k == t2.k);
    }
    /// @brief Not equal
    bool operator!=(sd_time const &t2) const { return !(*this == t2); }
    /// @brief Order by t then by k
    bool operator<(sd_time const &t2) const {
        return (t < t2.t || (t == t2.t && k < t2.k));
    }
    /// @brief Less than or equal
    bool operator<=(sd_time const &t2) const {
        return (*this == t2 || *this < t2);
    }
    /// @brief Greater than
    bool operator>(sd_time const &t2) const { return !(*this <= t2); }
    /// @brief Greater than or equal
    bool operator>=(sd_time const &t2) const { return !(*this < t2); }
    /// Advance this value by a step size t2
    sd_time operator+(sd_time const &t2) const {
        sd_time result(*this);
        result += t2;
        return result;
    }
    /// @brief Advance this value by a step size t2
    sd_time const &operator+=(sd_time const &t2) {
        if (t2.t == 0) {
            k += t2.k;
        } else {
            t += t2.t;
            k = t2.k;
        }
        return *this;
    }
    /// @brief Length of the interval from now to t2
    sd_time operator-(sd_time const &t2) const {
        sd_time result(*this);
        result -= t2;
        return result;
    }
    /// @brief Length of the interval from now to t2
    sd_time const &operator-=(sd_time const &t2) {
        if (t == t2.t) {
            t = 0;
            k -= t2.k;
        } else {
            t -= t2.t;
        }
        return *this;
    }
    /// @brief Print a time to the output stream
    friend std::ostream &operator<<(std::ostream &out, sd_time const &t) {
        out << "(" << t.t << "," << t.k << ")";
        return out;
    }
    /// @brief Read a time from the input stream
    friend std::istream &operator>>(std::istream &in, sd_time &t) {
        char junk;
        in >> junk >> t.t >> junk >> t.k >> junk;
        return in;
    }

  private:
    TimeType t;
    int k;
};

/** 
 * @brief This is an alternative double that may be used for the simulation clock
 * (i.e., as the template parameter TimeType for models and simulators).
 * 
 * This time type uses the fcmp function to check for equality instead of the
 * default equality operator. Information on the fcmp function
 * may be found at http://fcmp.sourceforge.net/
 * 
 * It may be useful for simulating with time granules as described in the article
 * <a href = "https://dl.acm.org/doi/10.1145/268823.268901">"The threshold of
 * event simultaneity"</a>.
 */
class double_fcmp {

  private:
    double d;

  public:
    /**
     * @brief A static variable that is the
     * tolerance for the fcmp function.
     * 
     * The user must instantiate this static variable
     * and initialize as required by the fcmp function.
     */
    static double epsilon;

    /// @brief Constructor initializes the value to the given argument.
    /// @param rhs The initial value. Default is zero.
    double_fcmp(double rhs = 0) : d(rhs) {}
    /// @brief Copy constructor
    double_fcmp(double_fcmp const &other) : d(other.d) {}
    /// @brief Assignment operator
    double_fcmp const &operator=(double_fcmp const &rhs) {
        d = rhs.d;
        return *this;
    }
    /// @brief Assignment operator
    double_fcmp const &operator=(double rhs) {
        d = rhs;
        return *this;
    }
    /// @brief Get the double value within the double_fcmp
    operator double() { return d; }
    /// @brief Less than
    bool operator<(double rhs) const { return (fcmp(d, rhs, epsilon) < 0); }
    /// @brief Less than
    bool operator<(double_fcmp const &rhs) const {
        return (fcmp(d, rhs.d, epsilon) < 0);
    }
    /// @brief Less than or equal to
    bool operator<=(double_fcmp const &rhs) const {
        return (fcmp(d, rhs.d, epsilon) <= 0);
    }
    /// @brief Greater than
    bool operator>(double_fcmp const &rhs) const {
        return (fcmp(d, rhs.d, epsilon) > 0);
    }
    /// @brief Greater than or equal to
    bool operator>=(double_fcmp const &rhs) const {
        return (fcmp(d, rhs.d, epsilon) >= 0);
    }
    /// @brief Equality
    bool operator==(double rhs) const { return (fcmp(d, rhs, epsilon) == 0); }
    /// @brief Equality
    bool operator==(double_fcmp const &rhs) const {
        return (fcmp(d, rhs.d, epsilon) == 0);
    }
    /// @brief Advance this value by a step size t2
    double_fcmp operator+(double_fcmp const &t2) const {
        return double_fcmp(d+t2.d);
    }
    /// @brief Advance this value by a step size t2
    double_fcmp const &operator+=(double_fcmp const &t2) {
        d += t2.d;
        return *this;
    }
    /// @brief Length of the interval from now to t2
    double_fcmp operator-(double_fcmp const &t2) const {
        return double_fcmp(d-t2.d);
    }
    /// @brief Assign length of the interval from now to t2
    double_fcmp const &operator-=(double_fcmp const &t2) {
        d -= t2.d;
        return *this;
    }

    /**
     * @brief A double value where equality is defined within a given tolerance.
     *
     * The fcmp() inline function is taken from fcmp.c, which is
     * Copyright (c) 1998-2000 Theodore C. Belding,
     * University of Michigan Center for the Study of Complex Systems,
     * <mailto:Ted.Belding@umich.edu>,
     * <http://www-personal.umich.edu/~streak/>,
     *
     * This code is part of the fcmp distribution. fcmp is free software;
     * you can redistribute and modify it under the terms of the GNU Library
     * General Public License (LGPL), version 2 or later.  This software
     * comes with absolutely no warranty. The implement is in adevs.cpp
     * 
     * @param x1 First double value to compare
     * @param x2 Second double value to compare
     * @param epsilon The tolerance for the comparison
     * @return  1 if x1 > x2, -1 if x1 < x2, and 0 if they are equal within the tolerance
     */
    static int fcmp(double x1, double x2, double epsilon);
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
