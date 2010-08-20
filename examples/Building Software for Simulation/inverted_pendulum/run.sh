OMNET_HOME=/usr/local/omnetpp-4.0
for i in 0 1 2 3 4 5 6 7 8 9 10;
do
	./controlsim -r $i -u Cmdenv -c General -n ./:${OMNET_HOME}/inet-framework/examples:${OMNET_HOME}/inet-framework/src -l ${OMNET_HOME}/inet-framework/src/inet ./omnetpp.ini
	mv s.dat results/s$i.dat
	mv y.dat results/y$i.dat
done
./mop results/s*.dat
