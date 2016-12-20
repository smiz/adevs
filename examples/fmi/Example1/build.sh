# Clean up stale files
rm -f a.out
rm -rf HelloWorld
omc sim_hello.mos
# Cleanup the junk produced by the omc compiler
rm -f *FMU* *.libs *.log *.json
# Unpack the fmu data 
mkdir HelloWorld
cd HelloWorld ; unzip -o -qq ../HelloWorld.fmu; cd ..
# Compile our simulator. You must have in the include path
#    The adevs include directory
#    The FMI for model exchange header files
# You must link to system library libdl
python ../../../util/xml2cpp.py -o HelloWorld -type double -f HelloWorld/binaries/linux64/HelloWorld.so -r HelloWorld/modelDescription.xml
g++ -Wall -O3 -I../../../include -I${HOME}/Code/FMI_for_ModelExchange_and_CoSimulation_v2.0 main_hello.cpp -ldl
# Run the model
./a.out

