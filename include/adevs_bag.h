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
#ifndef _adevs_bag_h
#define _adevs_bag_h
#include <cstdlib>

namespace adevs
{

/**
The Bag class is (almost) a model of a STL Multiple Associative Container.
This implementation is optimized for use by the simulation engine.
It does not satisfy the STL complexity requirements. Neither does it implement
the full set of required methods, but those methods that are implemented
conform to the standard (sans the complexity requirement).
*/
template <class T> class Bag
{
	public:
		class iterator
		{
			public:
				iterator(unsigned long int start = 0, T* b = NULL):
				i(start),b(b) 
				{ 
				}
				iterator(const iterator& src):
				i(src.i),b(src.b)
				{ 
				}
				const iterator& operator=(const iterator& src) 
				{ 
					i = src.i; 
					b = src.b;
					return *this;
				}
				bool operator==(const iterator& src) const
				{
					return i == src.i;
				}
				bool operator!=(const iterator& src) const
				{
					return i != src.i;
				}
				T& operator*() { return b[i]; }
				const T& operator*() const { return b[i]; }
				iterator& operator++() { i++; return *this; }
				iterator& operator--() { i--; return *this; }
				iterator& operator++(int) { ++i; return *this; }
				iterator& operator--(int) { --i; return *this; }
			private:
				friend class Bag<T>;	
				unsigned long int i;
				T* b;
		};

		typedef iterator const_iterator;

		Bag():
		cap_(10),
		size_(0)
		{
			b = new T[cap_];
		}
		Bag(const Bag<T>& src):
		cap_(src.cap_),
		size_(src.size_)
		{
			b = new T[src.cap_];
			for (unsigned long int i = 0; i < size_; i++) 
			{
				b[i] = src.b[i];
			}
		}
		const Bag<T>& operator=(const Bag<T>& src)
		{
			cap_ = src.cap_;
			size_ = src.size_;
			delete [] b;
			b = new T[src.cap_];
			for (unsigned long int i = 0; i < size_; i++) 
			{
				b[i] = src.b[i];
			}
			return *this;
		}
		unsigned count(const T& a) const
		{
			unsigned result = 0;
			for (unsigned i = 0; i < size_; i++)
			{
				if (b[i] == a) result++;
			}
			return result;
		}
		unsigned size() const 
		{ 
			return size_; 
		}
		bool empty() const
		{
			return size_ == 0;
		}
		iterator begin() const { return iterator(0,b); }
		iterator end() const { return iterator(size_,b); }
		void erase(const T& k) 
		{
			iterator p = find(k);
			if (p != end())
			{
				erase(p);
			}
		}
		void erase(iterator p)
		{
			size_--;
			b[p.i] = b[size_];
		}
		void clear()
		{
			size_ = 0;
		}
		iterator find(const T& k) const
		{
			for (unsigned i = 0; i < size_; i++)
			{
				if (b[i] == k)
				{
					return iterator(i,b);
				}
			}
			return end();
		}
		void insert(const T& t)
		{
			if (cap_ == size_) enlarge(2*cap_);
			b[size_] = t;
			size_++;
		}
		~Bag()
		{
			delete [] b;
		}
	
		private:	
			unsigned cap_;
			unsigned size_;
			T* b;

			/// Adds the specified capacity to the bag.
			void enlarge(unsigned adjustment)
			{
				cap_ = cap_ + adjustment;
				T* rb = new T[cap_];
				for (unsigned i = 0; i < size_; i++) 
				{
					rb[i] = b[i];
				}
				delete [] b;
				b = rb;
			}
	};

}

#endif
