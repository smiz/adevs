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
#ifndef __adevs_message_q_h_
#define __adevs_message_q_h_
#include "adevs_models.h"
#include "adevs_time.h"
#include <omp.h>
#include <list>
#include <cassert>

namespace adevs
{

template <typename X, class T> class LogicalProcess;

template <typename X, class T = double> struct Message
{
	typedef enum { OUTPUT, EIT } msg_type_t;
	Time<T> t;
	LogicalProcess<X,T> *src;
	Devs<X,T>* target;
	X value;
	msg_type_t type;
	// Default constructor
	Message():value(){}
	// Create a message with a particular value
	Message(const X& value):value(value){}
	// Copy constructor
	Message(const Message& other):
		t(other.t),
		src(other.src),
		target(other.target),
		value(other.value),
		type(other.type)
	{
	}
	// Assignment operator
	const Message<X,T>& operator=(const Message<X,T>& other)
	{
		t = other.t;
		src = other.src;
		target = other.target;
		value = other.value;
		type = other.type;
		return *this;
	}
	// Sort by time stamp, smallest time stamp first in the STL priority_queue
	bool operator<(const Message<X,T>& other) const
	{
		return other.t < t;
	}
	~Message(){}
};

template <class X, class T = double> class MessageQ
{
	public:
		MessageQ()
		{
			omp_init_lock(&lock);
			qshare_empty = true;
			qsafe = &q1;
			qshare = &q2;
		}
		void insert(const Message<X,T>& msg)
		{
			omp_set_lock(&lock);
			qshare->push_back(msg);
			qshare_empty = false;
			omp_unset_lock(&lock);
		}
		bool empty() const { return qsafe->empty() && qshare_empty; }
		Message<X,T> remove()
		{
			if (qsafe->empty())
			{
				std::list<Message<X,T> > *tmp = qshare;
				omp_set_lock(&lock);
				qshare = qsafe;
				qshare_empty = true;
				omp_unset_lock(&lock);
				qsafe = tmp;
			}
			Message<X,T> msg(qsafe->front());
			qsafe->pop_front();
			return msg;
		}
		~MessageQ()
		{
			omp_destroy_lock(&lock);
		}
	private:
		omp_lock_t lock;
		std::list<Message<X,T> > q1, q2;
		std::list<Message<X,T> > *qsafe, *qshare;
		volatile bool qshare_empty;
};

} // end of namespace

#endif
