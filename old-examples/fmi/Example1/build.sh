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
python3 ../../../util/xml2cpp.py -o HelloWorld -type double -f HelloWorld/binaries/linux64/HelloWorld.so -x HelloWorld/modelDescription.xml -r file://${PWD}/HelloWorld/resources
g++ -Wall -g -I../../../include -I${HOME}/Code/fmi/headers main_hello.cpp -ldl
# Run the model
./a.out

