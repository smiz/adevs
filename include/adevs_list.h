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
#ifndef _adevs_list_h
#define _adevs_list_h
#include <cstdlib>

namespace adevs
{

/**
 * This list is a model of an STL list, except that the individual
 * items that are stored in the list CAN ONLY BE ELEMENTS OF A SINGLE
 * LIST AT ANY TIME! This is an optimization for the OptSimulator that
 * was made because the STL list overheads were killing me. Items in
 * the list must be inheret the one_list interface which gives them
 * the proper pointers. The actual type stored in the list must be
 * a pointer to the template type.
 */
template <typename T> class list
{
	public:

		class one_list
		{
			public:
				virtual ~one_list(){}
			private:
				T *next, *prev;
			friend class adevs::list<T>;
		};
		/// A forward iterator for the list
		class iterator
		{
			public:
				iterator(T* start = NULL):i(start){}
				iterator(const iterator& src):i(src.i){}
				const iterator& operator=(const iterator& src) 
				{ 
					i = src.i; 
					return *this;
				}
				bool operator==(const iterator& src) const { return i==src.i; }
				bool operator!=(const iterator& src) const { return i!=src.i; }
				T* operator*() { return i; }
				const T* operator*() const { return i; }
				iterator& operator++() { i = i->next; return *this; }
				iterator& operator++(int) { i = i->next; return *this; }
			private:
				T* i;
			friend class adevs::list<T>;
		};
		typedef iterator const_iterator;
		list():head(NULL),tail(NULL){}
		bool empty() const { return head == NULL; }
		iterator begin() const { return iterator(head); }
		iterator end() const { return iterator(NULL); }
		// If free_list is not NULL, the removed it is pushed onto its front
		iterator erase(iterator pos, list<T>* free_list = NULL)
		{
			iterator return_val(pos.i->next);
			// Remove the only item in the list
			if (head == tail)
			{
				head = tail = NULL;
			}
			// Remove the head of the list
			else if (pos.i == head)
			{
				head = head->next;
				head->prev = NULL;
			}
			// Remove the tail of the list
			else if (pos.i == tail)
			{
				tail = tail->prev;
				tail->next = NULL;
			}
			// Otherwise remove from the end of the list
			else
			{
				pos.i->prev->next = pos.i->next;
				pos.i->next->prev = pos.i->prev;
			}
			if (free_list != NULL)
				free_list->push_front(pos.i);
			return return_val;
		}
		void insert(iterator pos, T* item)
		{
			// If this is the first item in the list
			if (head == NULL)
			{
				head = tail = item;
				item->next = item->prev = NULL;
			}
			// If this is the last item in the list
			else if (pos.i == NULL)
			{
				item->prev = tail;
				item->next = NULL;
				tail->next = item;
				tail = item;
			}
			// if it is to be the head of the list
			else if (pos.i == head)
			{
				item->next = head;
				item->prev = NULL;
				head->prev = item;
				head = item;
			}
			// otherwise insert it before the given position
			else
			{
				item->next = pos.i;
				item->prev = pos.i->prev;
				pos.i->prev->next = item;
				pos.i->prev = item;
			}
		}
		T* front() { return head; }
		const T* front() const { return head; }
		T* back() { return tail; }
		const T* back() const { return tail; }
		void push_front(T* item) { insert(iterator(head),item); }
		void push_back(T* item) { insert(iterator(NULL),item); }
		void pop_front(list<T>* free_list = NULL) { erase(iterator(head),free_list); }
		void pop_back(list<T>* free_list = NULL) { erase(iterator(tail),free_list); }
		~list(){}
	private:	
		T *head, *tail;
		// No copy constructor
		list(const list<T>& src){}
		// No assignment operator
		void operator=(const list<T>& src){}
};

} // end of namespace

#endif
