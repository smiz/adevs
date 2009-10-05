/***************
Copyright (C) 2008 by James Nutaro

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
#ifndef __adevs_time_h_
#define __adevs_time_h_
#include <cfloat>
#include <iostream>

namespace adevs
{

/// This is the super dense simulation clock
struct Time
{
	double t;
	unsigned int c;
	/// Value for infinity
	static adevs::Time Inf() { return Time(DBL_MAX,0); }
	/// Constructor. Default time is (0,0).
	Time(double t = 0.0, unsigned int c = 0):t(t),c(c){}
	/// Copy constructor
	Time(const Time& t2):t(t2.t),c(t2.c){}
	/// Assignment operator
	const Time& operator=(const Time& t2)
	{
		t = t2.t;
		c = t2.c;
		return *this;
	}
	/// Comparing with a double compares the real field
	bool operator<(double t2) const { return t < t2; }
	/// Assigning a double sets the real field to the double and the integer field to zero
	const Time& operator=(double t2)
	{
		t = t2;
		c = 0;
		return *this;
	}
	/// Advance operator (this is not commutative or associative!)
	Time operator+(const Time& t2) const
	{
	   if (t2.t == 0.0) return Time(t,t2.c+c);
	   else return Time(t+t2.t,0);
	}
	/// Advance and assign
	const Time& operator+=(const Time& t2)
	{
		*this = *this+t2;
		return *this;
	}
	/// Subtract a real number (used to get the elapsed time)
	double operator-(double t2) const
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

} // end namespace

std::ostream& operator<<(std::ostream& strm, const adevs::Time& t);

#endif
