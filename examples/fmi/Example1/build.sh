# Build the FMU 
omc sim_hello.mos
# Cleanup the junk produced by the omc compiler
rm -f HelloWorld_*
rm -f HelloWorld.c
# Unpack the fmu data 
unzip -o -qq HelloWorld.fmu
# Compile our simulator. You must have in the include path
#    The adevs include directory
#    The FMI for model exchange header files
# You must link to system library libdl
g++ -Wall -I../../../include -I${HOME}/Code/FMI_for_ModelExchange_and_CoSimulation_v2.0 main_hello.cpp -ldl
# Run the model
./a.out

