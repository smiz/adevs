# Clean up stale files
rm -f a.out
rm -rf TestCircuit
omc circuit.mos
# Cleanup the junk produced by the omc compiler
rm -f *FMU* *.libs *.log *.json
# Unpack the fmu data 
mkdir TestCircuit
unzip -d TestCircuit TestCircuit.fmu
# Compile our simulator. You must have in the include path
#    The adevs include directory
#    The FMI for model exchange header files
# You must link to system library libdl
python3 ../../util/xml2cpp.py -o TestCircuit -type double -f TestCircuit/binaries/linux64/TestCircuit.so -x TestCircuit/modelDescription.xml -r file://${PWD}/TestCircuit/resources
g++ -Wall -Ofast -I../../include -I${HOME}/Code/fmi/headers fmi.cpp -ldl -lsundials_kinsol

