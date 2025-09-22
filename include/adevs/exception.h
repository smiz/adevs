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

#ifndef _adevs_exception_h_
#define _adevs_exception_h_

#include <exception>
#include <string>


namespace adevs {

/**
 * @brief The exception class is used to report errors during a simulation.
 * 
 * The adevs::exception class is derived from the standard template
 * library exception class.
 */
class exception : public std::exception {
  public:
    /**
     * @brief Create an exception with an error message and, if appropriate,
     * a pointer to the model that created the error.
     * 
     * To avoid templated exceptions, the model pointer is just a void*.
     * @param msg The error message.
     * @param model A pointer to the model that created the error (optional).
     */
    exception(char const* msg, void* model = nullptr) : std::exception(), msg(msg), model(model) {}
    /// Copy constructor.
    exception(adevs::exception const &src) : std::exception(src), msg(src.msg), model(src.model) {}
    /// Get the error message.
    char const* what() const throw() { return msg.c_str(); }
    /// Get a pointer to the model that created the error.
    void* who() const { return model; }
    /// Destructor.
    ~exception() throw() {}

  private:
    std::string msg;
    void* model;
};

}  // namespace adevs

#endif
