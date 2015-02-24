# Build the robot FMU 
mode=quantized
omc sim_$mode.mos
# Cleanup the junk produced by the omc compiler
rm -f Robot_*
rm -f Robot.c
rm -f Robot.h
# Unpack the fmu data 
unzip -o -qq Robot.fmu
rm -f Robot.fmu
rm -f *.json
rm -rf sources
mv binaries robot
mv *.xml robot
# Build the controller fmu
omc sim_control.mos
rm -f Control_*
rm -f Control.c
rm -f Control.h
unzip -o -qq Control.fmu
rm -f Control.fmu
rm -f *.json
rm -rf sources
mv binaries control
mv *.xml control
# Compile our simulator. You must have in the include path
#    The adevs include directory
#    The FMI for model exchange header files
# You must link to system library libdl
g++ -g -D$mode \
	-Wall -I../../../include \
	-I${HOME}/Code/FMI_for_ModelExchange_and_CoSimulation_v2.0 \
	Ethernet.cpp main.cpp -ldl ../../../src/libadevs.a
# Run the model
#./a.out

