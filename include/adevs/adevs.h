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
/**
 * \mainpage
 * ADEVS is a C++ library for simulating discrete and hybrid dynamic systems.
 * It is based on the DEVS formalism, which is an offshoot of general sytems
 * theory. It is our hope that ADEVS will be useful for persons who do not
 * have prior experience with DEVS, but you may nonetheless find it helpful
 * to have at hand some useful references on the subject. The standard reference
 * is <a href="https://shop.elsevier.com/books/theory-of-modeling-and-simulation/zeigler/978-0-12-813370-5">
 * Theory of Modeling and Simulation</a> and you can find a great deal of
 * tutorial material from univerisities on the web. Apart from its primary use
 * as a tool for building simulation programs, the adevs library includes 
 * explicit support for using your simulation as a component in some other
 * simulation tool or program, simulating hybrid dynamic systems using the
 * <a href="https://fmi-standard.org/">FMI</a> model exchange standard,
 * and using CVode from the <a href="https://computing.llnl.gov/projects/sundials">SUNDIALS</a>
 * package to simulate cyber-physical systems.
 *
 */

/*
 * Include everything needed to compile user models.
 */

#ifndef _adevs_h_
#define _adevs_h_
#include "adevs/exception.h"
#include "adevs/models.h"
#include "adevs/simulator.h"
#include "adevs/solvers/event_locators.h"
#include "adevs/solvers/corrected_euler.h"
#include "adevs/solvers/hybrid.h"
#include "adevs/solvers/rk_45.h"
#endif