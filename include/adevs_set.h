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
The Set class should be a model of a STL Unique 
Associative Container.
*/
template <class T> class Set: public std::set<T>
{
};

/// Set intersection operator.  Returns A intersect B.
template <class T> 
Set<T> set_intersect(const Set<T>& A, const Set<T>& B)
{
	Set<T> result;
	std::insert_iterator<Set<T> > i_iter(result,result.begin());
	std::set_intersection(A.begin(),A.end(),B.begin(),B.end(),i_iter);
	return result;
}
/// Set difference operator. Returns the set A-B.
template <class T> 
Set<T> set_difference(const Set<T>& A, const Set<T>& B)
{
	Set<T> result;
	std::insert_iterator<Set<T> > i_iter(result,result.begin());
	std::set_difference(A.begin(),A.end(),B.begin(),B.end(),i_iter);
	return result;
}
/// Set union operator.  Assigns A union B to the set A.
template <class T> 
void set_assign_union(Set<T>& A, const Set<T>& B)
{
	typename Set<T>::const_iterator iter = B.begin();
	for (; iter != B.end(); iter++)
	{
		A.insert(*iter);
	}
}

}

#endif
