# Compiler settings. Edit these as needed.
CFLAGS = -Wall -O3
CC = g++

# Adjust these as needed to find the adevs headers
INCLUDE = -I../../../include 
LIBS = -llapack -lblas 

##
# You should not need to edit below this line
##

OBJS = \
	AdmittanceNetwork.o \
	BusFreqListener.o \
	BusVoltsListener.o \
	ElectricalData.o \
	ElectricalModel.o \
	ElectricalModelEqns.o \
	GenrFail.o \
	GenrFreqListener.o \
	IEEE118.o \
	IEEE_CDF_Data.o \
	LoadControl.o \
	MechPowerListener.o \
	QueueBus.o \
	main.o

.SUFFIXES: .cpp
.cpp.o:
	${CC} ${CFLAGS} ${INCLUDE} -c $<

sim: ${OBJS}
	${CC} ${CFLAGS} ${OBJS} ${LIBS}

clean:
	rm -f *.o core a.out *.a *.dat
