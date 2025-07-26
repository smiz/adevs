This example simulates an inverter based control
for harmonic compensation in an electrical power
system. This example uses OpenModelica to model
the continous elements of the circuit. The switching
elements can be modeled with OpenModelica or in
adevs. You can compare the performance of the adevs
solvers with and output state events and the DASSL
solver that is native to OpenModelica (the latter
having an advantage in not suffering from FMI
overheads).
