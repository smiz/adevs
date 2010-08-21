# Build the ideal simulator and MOP calculator
make -f Makefile.ideal 
# These should point to your OMNET++ installation.
OMNET_HOME=/usr/local/omnetpp-4.1
INET_PROJ=$OMNET_HOME/inet
# This constructs the makefile for your OMNET++ simulator
opp_makemake -f -e cc \
-I../../../include \
-I$INET_PROJ/src/linklayer/ethernet \
-I$INET_PROJ/src/networklayer/ipv4 \
-I$INET_PROJ/src/networklayer/common \
-I$INET_PROJ/src/networklayer/rsvp_te \
-I$INET_PROJ/src/networklayer/icmpv6 \
-I$INET_PROJ/src/transport/tcp \
-I$INET_PROJ/src/base \
-I$INET_PROJ/src/networklayer/mpls \
-I$INET_PROJ/src/networklayer/ted \
-I$INET_PROJ/src/util/headerserializers \
-I$INET_PROJ/src/networklayer/contract \
-I$INET_PROJ/src/util \
-I$INET_PROJ/src/transport/contract \
-I$INET_PROJ/src/transport/sctp \
-I$INET_PROJ/src/networklayer/ipv6 \
-I$INET_PROJ/src/linklayer/mfcore \
-I$INET_PROJ/src/world \
-I$INET_PROJ/src/applications/pingapp \
-I$INET_PROJ/src/linklayer/contract \
-I$INET_PROJ/src/networklayer/arp \
-I$INET_PROJ/src/networklayer/ldp \
-I$INET_PROJ/src/transport/udp \
-L$INET_PROJ/out/'$(CONFIGNAME)'/src \
-linet -KINET_PROJ=$INET_PROJ
# Build the simulator
make 
