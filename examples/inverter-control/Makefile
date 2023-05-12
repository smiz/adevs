# GNU gcc compiler
CFLAGS = -Ofast -Wall -I../../include 
CC = g++

all: control_sim control_opt

control_sim:
	${CC} ${CFLAGS} -o sim circuit.cpp control.cpp pwm.cpp harmonic_compensator.cpp sim.cpp -llapack -lblas

control_opt:
	${CC} ${CFLAGS} -o opt -fopenmp circuit.cpp control.cpp pwm.cpp harmonic_compensator.cpp opt.cpp -llapack -lblas -lfftw3

pwm_test_2:
	${CC} ${CFLAGS} pwm.cpp pwm_test_2.cpp 

pwm_test:
	${CC} ${CFLAGS} circuit.cpp pwm.cpp pwm_test.cpp -llapack -lblas

circuit_test:
	${CC} ${CFLAGS} circuit.cpp circuit_test.cpp -llapack -lblas

clean:
	rm -f sim opt
