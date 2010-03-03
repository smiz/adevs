PREFIX = ../..
include ../make.common

check: atomic ping dynstruct

atomic:
	javac AtomicTest.java
	java AtomicTest > tmp
	diff -b AtomicTest.ok tmp

ping:
	javac PingPong.java
	java PingPong > tmp
	tail -5 tmp
	echo "These numbers should not grow by much"

dynstruct:
	javac DynStructPingPong.java
	java DynStructPingPong > tmp
	diff -b DynStructPingPong.ok tmp
