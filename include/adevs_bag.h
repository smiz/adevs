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
#ifndef _adevs_bag_h
#define _adevs_bag_h
#include <cstdlib>

namespace adevs
{

/**
 * The Bag is (almost) a model of a STL Multiple Associative Container.
 * This implementation is optimized for the simulation engine, and it 
 * does not satisfy the STL complexity requirements. Neither does it implement
 * the full set of required methods, but those methods that are implemented
 * conform to the standard (except for the time complexity requirement).
 */
template <class T> class Bag
{
	public:
		/// A bidirectional iterator for the Bag
		class iterator
		{
			public:
				iterator(unsigned int start = 0, T* b = NULL):
				i(start),b(b){} 
				iterator(const iterator& src):
				i(src.i),b(src.b){}
				const iterator& operator=(const iterator& src) 
				{ 
					i = src.i; 
					b = src.b;
					return *this;
				}
				bool operator==(const iterator& src) const { return i==src.i; }
				bool operator!=(const iterator& src) const { return i!=src.i; }
				T& operator*() { return b[i]; }
				const T& operator*() const { return b[i]; }
				iterator& operator++() { i++; return *this; }
				iterator& operator--() { i--; return *this; }
				iterator& operator++(int) { ++i; return *this; }
				iterator& operator--(int) { --i; return *this; }
			private:
				friend class Bag<T>;	
				unsigned int i;
				T* b;
		};
		typedef iterator const_iterator;
		/// Create an empty bag with an initial capacity
		Bag(unsigned int cap = 8):
		cap_(cap),size_(0),b(new T[cap]){}
		/// Copy constructor uses the = operator of T
		Bag(const Bag<T>& src):
		cap_(src.cap_),
		size_(src.size_)
		{
			b = new T[src.cap_];
			for (unsigned int i = 0; i < size_; i++) 
				b[i] = src.b[i];
		}
		/// Assignment operator uses the = operator of T
		const Bag<T>& operator=(const Bag<T>& src)
		{
			cap_ = src.cap_;
			size_ = src.size_;
			delete [] b;
			b = new T[src.cap_];
			for (unsigned int i = 0; i < size_; i++) 
				b[i] = src.b[i];
			return *this;
		}
		/// Swaps contents of this bag with the contents of the supplied bag. Returns this bag.
		Bag<T>& swap(Bag<T>& src)
		{
			unsigned tmp_cap_, tmp_size_;
			T* tmp_b;
			tmp_cap_ = src.cap_;
			tmp_size_ = src.size_;
			tmp_b = src.b;
			src.cap_ = cap_;
			src.size_ = size_;
			src.b = b;
			cap_ = tmp_cap_;
			size_ = tmp_size_;
			b = tmp_b;
			return *this;
		}
		/// Count the instances of a stored in the bag
		unsigned count(const T& a) const
		{
			unsigned result = 0;
			for (unsigned i = 0; i < size_; i++)
				if (b[i] == a) result++;
			return result;
		}
		/// Get the number of elements in the bag
		unsigned size() const { return size_; }
		/// Same as size()==0
		bool empty() const { return size_ == 0; }
		/// Get an iterator pointing to the first element in the bag
		iterator begin() const { return iterator(0,b); }
		/// Get an iterator to the end of the bag (i.e., just after the last element)
		iterator end() const { return iterator(size_,b); }
		/// Erase the first instance of k
		void erase(const T& k) 
		{
			iterator p = find(k);
			if (p != end()) erase(p);
		}
		/// Erase the element pointed to by p
		void erase(iterator p)
		{
			size_--;
			b[p.i] = b[size_];
		}
		/// Remove all of the elements from the bag
		void clear() { size_ = 0; }
		/// Find the first instance of k, or end() if no instance is found. Uses == for comparing T.
		iterator find(const T& k) const
		{
			for (unsigned i = 0; i < size_; i++)
				if (b[i] == k) return iterator(i,b);
			return end();
		}
		/// Put t into the bag
		void insert(const T& t)
		{
			if (cap_ == size_) enlarge(2*cap_);
			b[size_] = t;
			size_++;
		}
		~Bag() { delete [] b; }
	private:	
		unsigned cap_, size_;
		T* b;
		/// Adds the specified capacity to the bag.
		void enlarge(unsigned adjustment)
		{
			cap_ = cap_ + adjustment;
			T* rb = new T[cap_];
			for (unsigned i = 0; i < size_; i++) 
				rb[i] = b[i];
			delete [] b;
			b = rb;
		}
	};

} // end of namespace

#endif
