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

#ifndef _adevs_exception_h_
#define _adevs_exception_h_
#include <string>
#include <exception>

namespace adevs
{

/**
The adevs::exception class is derived from the standard template
library exception class.
*/
class exception: public std::exception
{
	public:
		/**
		Create an exception with an error message and, if appropriate,
		a pointer to the model that created the error.  To avoid
		templated exceptions, the model pointer is just a void*.
		*/
		exception(const char* msg, void* model = NULL):
		std::exception(),
		msg(msg),
		model(model) 
		{}
		/**
		Copy constructor.
		*/
		exception(const adevs::exception& src):
		std::exception(src),
		msg(src.msg),
		model(src.model)
		{}
		/**
		Get the error message.
		*/
		const char* what() const throw()
		{
			return msg.c_str();
		}
		/**
		Get a pointer to the model that created the error.
		*/
		void* who() const { return model; }
		/**
		Destructor.
		*/
		~exception() throw(){}
	private:
		std::string msg;
		void* model;
};

} // end of namespace

#endif

