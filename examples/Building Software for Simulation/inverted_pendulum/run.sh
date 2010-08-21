OMNET_HOME=/usr/local/omnetpp-4.1
INET_HOME=${OMNET_HOME}/inet
for i in {0..10};
do
	./inverted_pendulum -r $i -u Cmdenv -c General -n "${PWD}":${INET_HOME}/src:${INET_HOME}/examples -l ${INET_HOME}/src/inet ./omnetpp.ini
	mv s.dat results/s$i.dat
	mv y.dat results/y$i.dat
done
./mop results/s*.dat
