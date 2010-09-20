/***************
Copyright (C) 2000-2006 by James Nutaro

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
#ifndef _adevs_set_h
#define _adevs_set_h
#include <set>
#include <algorithm>

namespace adevs
{

/**
 * This Set is just an STL set. 
 */
template <class T> class Set: public std::set<T>
{
};

/// Set difference operator. Returns the set A-B.
template <class T> 
void set_assign_diff(Bag<T>& result, const Set<T>& A, const Set<T>& B)
{
	typename Set<T>::const_iterator iter = A.begin();
	for (; iter != A.end(); iter++)
	{
		if (B.find(*iter) == B.end()) result.insert(*iter);
	}
}

} // end of namespace

#endif
