# Build the FMU 
omc makefmi.mos
# Unpack the fmu data 
mkdir CherryBomb
cd CherryBomb ; unzip -o -qq ../CherryBomb.fmu ; cd ..
rm -f *FMU*
rm -f *.fmu
rm *.log
rm *.libs
rm *.h
rm *.json
# Compile our simulator. You must have in the include path
#    The adevs include directory
#    The FMI for model exchange header files
# You must link to system library libdl
python3 ../../../util/xml2cpp.py -o CherryBomb -type std::string -f CherryBomb/binaries/linux64/CherryBomb.so -x CherryBomb/modelDescription.xml -r file://${PWD}/CherryBomb/resources
g++ -Wall -I../../../include -I${HOME}/Code/fmi/headers main.cpp -ldl
# Run the model
./a.out

