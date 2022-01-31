# Cleanup stale files
rm -rf Sampled
rm -rf Quantized
rm -rf Control
rm -f a.out
# Build FMU's
omc sim_control.mos
mkdir Control
cd Control ; unzip -o -qq ../Control.fmu; cd ..
# Cleanup the junk produced by the omc compiler
rm -f *FMU* *.fmu *.log *.json *.libs
# Unpack the fmu data 
omc sim_quantized.mos
mkdir Quantized
cd Quantized ; unzip -o -qq ../Robot.fmu ; cd ..
#rm -f *FMU* *.fmu *.log *.json *.libs
omc sim_sampled.mos
mkdir Sampled
cd Sampled ; unzip -o -qq ../Robot.fmu ; cd ..
#rm -f *FMU* *.fmu *.log *.json *.libs
# Compile our simulator. You must have in the include path
#    The adevs include directory
#    The FMI for model exchange header files
# You must link to system library libdl
python3 ../../../util/xml2cpp.py -o Control -type IO_Type -f Control/binaries/linux64/Control.so -x Control/modelDescription.xml -r file://${PWD}/Control/resources
# Build the quantized model. Change this to build the Sampled model.
#python3 ../../../util/xml2cpp.py -o Robot -type IO_Type -f Quantized/binaries/linux64/Robot.so -x Quantized/modelDescription.xml -r file://${PWD}/Quantized/resources
python3 ../../../util/xml2cpp.py -o Robot -type IO_Type -f Sampled/binaries/linux64/Robot.so -x Sampled/modelDescription.xml -r file://${PWD}/Sampled/resources
g++ -g \
	-Wall -I../../../include \
	-I${HOME}/Code/fmi/headers \
	Ethernet.cpp main.cpp -ldl ../../../src/libadevs.a
# Run the model
./a.out

