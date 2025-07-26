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
 * theory. It is our hope that adevs will be useful to persons who do not
 * have prior experience with DEVS, but you may nonetheless find it helpful
 * to have at hand references on the subject. The standard reference
 * is <a href="https://shop.elsevier.com/books/theory-of-modeling-and-simulation/zeigler/978-0-12-813370-5">
 * Theory of Modeling and Simulation</a>. You might also be interested in
 * <a href="https://www.wiley.com/en-us/Building+Software+for+Simulation%3A+Theory+and+Algorithms%2C+with+Applications+in+C%2B%2B-p-9781118099452">
 * Building Software for Simulation</a>, which introduces the DEVS formalism
 * through a previous version of adevs. Another excellent reference is
 * <a href="https://www.routledge.com/Discrete-Event-Modeling-and-Simulation-A-Practitioners-Approach/Wainer/p/book/9781420053364?srsltid=AfmBOooqmd8In3-qK3DojTjMkWKb6IBehk0BHt2HAeooIs24Sifi7cFE">Discrete-Event Modeling and Simulation: A Practitioner's Approach</a>.
 * You can also find a great deal of
 * tutorial material on university websites, such as these
 * 
 * - <a href="https://simulation.tudelft.nl/SEN9110/lectures/04%20VanTendelooVangheluwe_SpringSim2017_DEVSTutorial.pdf">
 * Introduction to parallel DEVS modeling and simulation</a>
 * - <a href="http://msdl.uantwerpen.be/projects/DEVS/PythonPDEVS/Tutorial/18.WinterSim.DEVSTutorial.pdf">
 * Discrete Event System Specification (DEVS) Modelling and Simulation</a>
 * - <a href="https://www.cs.csi.cuny.edu/~gu/teaching/courses/csc754/slides/ACIMS%20DEVSTut%20AIS2002%20Final.pdf">
 * DEVS component based M\&S framework: an introduction</a>
 * - <a href="https://arslab.sce.carleton.ca/index.php/devs-tools/">DEVS Tools</a>. This is not a tutorial but
 * a list of other DEVS based simulation tools that you might be interested in.
 
 * Apart from the primary use of adevs
 * as a tool for building simulation programs, the adevs library includes 
 * explicit support for using your simulation as a component in some other
 * simulation tool or program, for simulating hybrid dynamic systems using the
 * <a href="https://fmi-standard.org/">FMI</a> model exchange standard,
 * and for using CVode from the <a href="https://computing.llnl.gov/projects/sundials">SUNDIALS</a>
 * package to simulate cyber-physical systems.
 * Before looking at the tutorials, you may want to examine the documentation
 * for the adevs::Atomic and adevs::Simulator classes. These are the fundamental
 * building blocks of your simulation program.
 * 
 * Here is a first example of a simulation program that uses the adevs::Atomic and
 * adevs::Simulator classes. This example presents a simple model derived from adevs::Atomic.
 * This simple model produces output and undergoes a change of state and regular intervals.
 *
 * \subpage ex1
 * 
 * In this next example we have two components. The first is similar to the Periodic
 * class in our previous example. It differs in placing an adevs::PinValue object
 * into the list supplied to the output function. The second component receives
 * this adevs::PinValue object as an input via its external transition function.
 * Let us call the first Atomic model 'A', the second 'B', and the adevs::pin_t that links
 * 'A' to 'B' we will call 'p'. The adevs::Graph class is used to form the connections shown
 * below:
 * 
 * \dot
 * digraph {
 *  rankdir="LR";
 *  node [shape="plaintext"]
 *  A -> p [style="dashed"];
 *  p -> B;
 * }
 * \enddot
 *
 * The dashed connection in the graph is created implicitly when the output
 * function of a 'A' puts a PinValue object with pin 'p' into the list passed
 * to the output function. The solid connection is created by calling the
 * graph method
 * \verbatim
   graph->connect(p,B);
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
 *
 * Our next example simulates a pipelined computer processor core. This example
 * demonstrates how the adevs::Coupled class is used to create hierarchical
 * models. It also demonstates the use of a adevs::MealyAtomic to create a model that
 * produces output in direct response to an input. The components of a model with
 * a single processor are shown in the graph below. The adevs::Atomic models are displayed in boxes.
 * The adevs::pin_t objects are displayed without an outline. Comparing the graph that
 * is displayed with the code in the example, you will see that the create_coupling() method
 * of the adevs::Coupled class acts just as the connect() method of the adevs::Graph class.
 *
 * The adevs::MealyAtomic in this example is the InstructionSource. It produces an instruction
 * for the Processor to execute whenever the processor indicates it is ready by issuing
 * an event on the ready_to_receive pin. The Processor constains three stages. Stage 0
 * fetches an instruction. State 1 decodes the instruction. Stage 2 executs the instruction.
 * In this example we can create a computer with multiple Processor's by creating several
 * instances of the Processor, which is an instance of an adevs::Coupled model.
 * 
 * \subpage ex4diagram
 * 
 * \subpage ex4
 *
 * This next example shows how to use the adevs::Hybrid class to simulate a model with a component
 * that has a piecewise continuous dynamic. The adevs::ode_system class is used to define
 * the dynamic behavior. The virtual methods of the adevs::ode_system are overriden to 
 * define the derivative function, state event function, output function, and discrete
 * state transition functions. The adevs::Hybrid class is derived from the adevs::Atomic class,
 * and it implements a numerical solver for the equations defined by the adevs::ode_system class.
 * The adevs::ode_system is supplied to the adevs::Hybrid object to define the model's
 * dynamic behavior. Then the adevs::Hybrid object is added to your model just like an other
 * adevs::Atomic component.
 * 
 * \subpage ex5plot
 * 
 * \subpage ex5
 *
 * The test case below illustrates how components and couplings can be added to and removed
 * from an adevs::Coupled model within a running simulation. This example very
 * closely resembles the <a href="https://dl.acm.org/doi/10.1145/224401.224731">
 * Dynamic Structure DEVS</a> approach to organizing a simulation model with components
 * and couplings that change in time.
 * 
 * \subpage ex6
 * 
 * Below are several miscellaneous example.
 *
 * The \subpage game_of_life is a famous cellular automaton. You can read about it
 * <a href="https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life">here</a>.
 */

 /**
  * \page game_of_life Game of Life
  * \include game_of_life.cpp
  */

 /**
 * \page ex1 Example #1
 * \include ex1.cpp
 */

/**
 * \page ex2 Example #2
 * \include ex2.cpp
 */

/**
 * \page ex3 Example #3
 * \include ex3.cpp
 */
 
/**
 * \page ex4diagram Example #4 diagram
 * \dot
 * digraph {
 *  {
 *   node [shape="box",fontsize="11"]
 *   InstructionSource
 *   Stage_0
 *   Stage_1
 *   Stage_2
 *  }
 *  {
 *   node [shape="plaintext",fontsize="10"]
 *   receive_instruction_0
 *   receive_instruction_1
 *   receive_instruction_2
 *   receive_instruction
 *   execute_instruction
 *   ready_to_receive_0
 *   ready_to_receive_1
 *   ready_to_receive_2
 *   ready_to_receive
 *   ready_to_transmit_0
 *   ready_to_transmit_1
 *   transmit_instruction_0
 *   transmit_instruction_1
 *  }
 *  subgraph  cluster_0
 *  {
 *    label="Processor";
 *    ready_to_receive_0 -> ready_to_receive;
 *    Stage_0 -> ready_to_receive_0 [style="dashed"];
 *    Stage_1 -> ready_to_receive_1 [style="dashed"];
 *    Stage_2 -> ready_to_receive_2 [style="dashed"];
 *    receive_instruction -> receive_instruction_0;
 *    receive_instruction_0 -> Stage_0;
 *    receive_instruction_1 -> Stage_1;
 *    receive_instruction_2 -> Stage_2;
 *    ready_to_receive_1 -> ready_to_transmit_0;
 *    ready_to_receive_2 -> ready_to_transmit_1;
 *    ready_to_transmit_0 -> Stage_0;
 *    ready_to_transmit_1 -> Stage_1;
 *    Stage_0 -> transmit_instruction_0 [style="dashed"];
 *    Stage_1 -> transmit_instruction_1 [style="dashed"];
 *    transmit_instruction_0 -> receive_instruction_1;
 *    transmit_instruction_1 -> receive_instruction_2;
 *  }
 *  InstructionSource -> execute_instruction [style="dashed"];
 *  execute_instruction -> receive_instruction;
 *  ready_to_receive -> InstructionSource;
 * }
 * \enddot
 */

/**
 * \page ex4 Example #4
 * \include ex4.cpp
 * 
 * \page ex5plot Voltage, diode state, and switch state vs time
 * \image html circuit.png
 *
 * \page ex5 Example #5
 * \include ex5.cpp
 * 
 * \page ex6 Example #6
 * \include coupled_test.cpp
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