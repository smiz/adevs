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
 * adevs is a C++ library for simulating discrete and hybrid dynamic systems.
 * It is based on the DEVS formalism, which is an offshoot of general systems
 * theory. It is our hope that adevs will be useful for persons who do not
 * have prior experience with DEVS, but you may nonetheless find it helpful
 * to have at hand some useful references on the subject. The standard reference
 * is <a href="https://shop.elsevier.com/books/theory-of-modeling-and-simulation/zeigler/978-0-12-813370-5">
 * Theory of Modeling and Simulation</a> and you can find a great deal of
 * tutorial material from universities on the web. Apart from its primary use
 * as a tool for building simulation programs, the adevs library includes 
 * explicit support for using your simulation as a component in some other
 * simulation tool or program, simulating hybrid dynamic systems using the
 * <a href="https://fmi-standard.org/">FMI</a> model exchange standard,
 * and using CVode from the <a href="https://computing.llnl.gov/projects/sundials">SUNDIALS</a>
 * package to simulate cyber-physical systems.
 * 
 * Before looking at the tutorials, you may want to examine the document
 * for the adevs::Atomic and adevs::Simulator classes. These are the fundamental
 * building blocks of your simulation program. Here is a first example of a
 * simulation program that uses these classes.
 *
 * \subpage ex1
 * 
 * In this next example we have two components. The first is similar to the Periodic
 * class in our previous example. It differs in placing an adevs::PinValue object
 * into the list supplied to the output function. The second component receives
 * this adevs::PinValue object as an input via its external transition function.
 * Let us call the first component A, the second B, and the adevs::pin_t that links
 * A to B we will call p. The adevs::Graph class is used to form the connections as shown
 * below:
   \verbatim
  
   A --> p --> B

   \endverbatim
 * The source code for this simulation program is shown below.
 * 
 * \subpage ex2
 * 
 * In our third example we see the implementation of a typical queuing simulation.
 * One adevs::Atomic component simulates the arrival of jobs (e.g., customers at
 * a fast food restaurant) in time. A second adevs::Atomic component simulates the
 * queue that holds jobs waiting to be processed and the server that does the
 * processing (e.g., the line and clerk at the counter). This is the first example
 * that exercises every aspect of the adevs::Atomic interface.
 * 
 * \subpage ex3
 */

 /**
 * \page ex1 Example #1
 * \verbinclude ex1.cpp
 */

/**
 * \page ex2 Example #2
 * \verbinclude ex2.cpp
 */

/**
 * \page ex3 Example #3
 * \verbinclude ex3.cpp
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