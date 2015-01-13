This directory contains several examples of how to use the FMI
import feature. These examples were built using the OpenModelica
compiler and its FMI2 export facilities.

To run these examples you will need OpenModelica and the FMI
for model exchange header files. You can get these from

http://www.openmodelica.org
https://www.fmi-standard.org/downloads#version2

You will need to edit the build scripts in each example to
point to your installation of the FMI2 header files.

Example1
--------
A simple example with the single dx/dt = a*x.

Exampl2
-------
This is the model of a robot controlled through an ethernet
network that is described in J. Nutaro, "An extension of the
OpenModelica compiler for using Modelica models in a discrete
event simulation," SIMULATION, vol. 90, pp. 1328-1345,
December 2014.

