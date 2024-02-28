# Clean up stale files
rm -f a.out
rm -rf TestCircuit*
omc circuit.mos
# Cleanup the junk produced by the omc compiler
rm -f *FMU* *.libs *.log *.json
# Build each FMU
fmus="TestCircuitModelica TestCircuitAdevs"
for fmu in $fmus
do
	echo $fmu
	# Unpack the fmu data 
	mkdir $fmu
	unzip -d $fmu $fmu.fmu
	# Create The FMI for model exchange header files
	python3 ../../util/xml2cpp.py -o $fmu -type IOType -f $fmu/binaries/linux64/$fmu.so -x $fmu/modelDescription.xml -r file://${PWD}/$fmu/resources
done
# Build the executable
g++ -Wall -Ofast -I../../include -I${HOME}/Code/fmi/headers fmi.cpp -ldl -lsundials_kinsol

