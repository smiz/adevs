# GNU gcc compiler
CC = g++
CFLAGS = -g -Wall -std=c++17
FMI_HOME = $(HOME)/Code/libfmi/install
CFLAGS += -I$(FMI_HOME)/include -I../../include
LIBS += -L../.. -L$(FMI_HOME)/lib -ladevs -lfmilib -lsundials_kinsol
TEST_EXEC = ./a.out

all:
	omc make_fmu.mos
	$(CC) $(CFLAGS) circuit.cpp $(LIBS)

clean:
	rm -f a.out *.fmu
