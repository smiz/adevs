CC=g++
FLAGS=-Ofast -Wall -c
INCLUDE=-I${PWD}/include

all: lib docs

clean:
	rm -f libadevs.a
	rm src/adevs.o
	rm -rf html

lib:
	$(CC) $(FLAGS) $(INCLUDE) src/adevs.cpp -o src/adevs.o 
	ar rvs libadevs.a src/adevs.o

docs:
	doxygen doxygen.config
