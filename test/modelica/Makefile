PREFIX=../..
include ../make.common
# GNU gcc compiler
SUNDIALS=/usr/local/sundials
OMHOME=$(HOME)/Code/openmodelica/trunk/build
OMLIB=$(OMHOME)/lib/omc/libf2c.a -L$(OMHOME)/lib/omc
CFLAGS += -I${SUNDIALS}/include -I$(OMHOME)/include/omc
LIBS += $(OMLIB) -L${SUNDIALS}/lib -lgc -lsundials_kinsol -lsundials_nvecserial

#check: te tg t0 t1 t2 t3 t4 t5 t6 t7 linsys linsys2 dae1 event_iter delayed_sir hi
check: tg t0 t1 t2 t3 t4 t5 t6 t7 linsys linsys2 dae1 event_iter delayed_sir hi

dae1:
	omc +s +simCodeTarget=Adevs DAEexample.mo
	$(CC) $(CFLAGS) main_daeexample.cpp DAEexample.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) DAEexample.cpp DAEexample.h

delayed_sir:
	omc +s +simCodeTarget=Adevs DelayedSIR.mo
	$(CC) $(CFLAGS) main_influenza.cpp Influenza.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	diff tmp delayed_sir.ok
	$(DEL) Influenza.cpp Influenza.h

event_iter:
	omc +s +simCodeTarget=Adevs eventIter.mo 
	$(CC) $(CFLAGS) eventIter.cpp main_eventIter.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) eventIter.cpp eventIter.h

hi:
	omc +s +simCodeTarget=Adevs HighIndex.mo 
	omc +s +simCodeTarget=Adevs HighIndexControl.mo 
	$(CC) $(CFLAGS) Control.cpp Robot.cpp main_highIndex.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) Robot.cpp Robot.h
	$(DEL) Control.cpp Control.h

linsys2:
	omc +s +simCodeTarget=Adevs Linsys2.mo 
	$(CC) $(CFLAGS) Linsys2.cpp Linsys2_main.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) Linsys2.cpp Linsys2.h

linsys:
	omc +s +simCodeTarget=Adevs Linsys.mo 
	$(CC) $(CFLAGS) Linsys.cpp Linsys_main.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) Linsys.cpp Linsys.h

tg:
	omc +s +simCodeTarget=Adevs builtins-events-algs.mo 
	$(CC) $(CFLAGS) builtins_events.cpp main_builtins_algs.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) builtins_events.cpp builtins_events.h

te:
	omc +s +simCodeTarget=Adevs builtins-events.mo 
	$(CC) $(CFLAGS) builtins_events.cpp main_builtins_events.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) builtins_events.cpp builtins_events.h

t0:
	omc +s +simCodeTarget=Adevs builtins-noevents.mo 
	$(CC) $(CFLAGS) builtins_noevents.cpp main_builtins_noevents.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) builtins_noevents.cpp builtins_noevents.h

t1:
	omc +s +simCodeTarget=Adevs test1.mo 
	$(CC) $(CFLAGS) test1.cpp main_test1.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) test1.cpp test1.h

t2:
	omc +s +simCodeTarget=Adevs test2.mo 
	$(CC) $(CFLAGS) test2.cpp main_test2.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) test2.cpp test2.h

t3:
	omc +s +simCodeTarget=Adevs test3.mo 
	$(CC) $(CFLAGS) Circuit.cpp main_test3.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) Circuit.cpp Circuit.h

t4:
	omc +s +simCodeTarget=Adevs test4.mo 
	$(CC) $(CFLAGS) Circuit.cpp main_test4.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) Circuit.cpp Circuit.h

t5:
	omc +s +simCodeTarget=Adevs EventTests.mo 
	$(CC) $(CFLAGS) EventTests.cpp main_test5.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) EventTests.cpp EventTests.h

t6:
	omc +s +simCodeTarget=Adevs SimplePeriodicSampler.mo 
	$(CC) $(CFLAGS) Sampler.cpp SimplePeriodicSampler_main.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) Sampler.cpp Sampler.h
	
t7:
	omc +s +simCodeTarget=Adevs Delay.mo 
	$(CC) $(CFLAGS) Delay.cpp main_delay.cpp $(LIBS)
	$(TEST_EXEC) > tmp
	$(DEL) Delay.cpp Delay.h
