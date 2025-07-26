tank_common_cpp = [
    'TankPhysics.cpp',
    'TankPhysicsEqns.cpp',
    'InterruptHandler.cpp',
    'PacketProcessing.cpp',
    'Computer.cpp',
    'Tank.cpp',
    'Display.cpp',
    'SimControl.cpp',
]

tank_common = static_library(
    'tank_common',
    tank_common_cpp,
    include_directories: adevs,
    gnu_symbol_visibility: 'hidden',
)

# Interactive simulation used in Chapter 2.
executable('sim', 'main.cpp', include_directories: adevs, link_with: tank_common)

# Simulator for the lost power experiment in section 2.4.
executable('exp', 'constInputExp.cpp', include_directories: adevs, link_with: tank_common)

# Simulator for the computer. This is discussed in section 4.4.
executable('comp', 'computerTest.cpp', include_directories: adevs, link_with: tank_common)

# Simulator for testing single manuevers
executable('tank', 'tankTest.cpp', include_directories: adevs, link_with: tank_common)

# LIBS = -lSDL -lglut -lccgnu2

# # Build the InterruptHandler simulation. This code is discussed in section 4.1.9.
# ih: InterruptHandler.o interruptExperiment.o
# 	${CC} ${CFLAGS} ${OPTFLAG} interruptExperiment.o InterruptHandler.o ${LIBS} ${LIBPATH} ${INCLUDE}

# modelica: ${BASE_OBJS} modelica_test.o
# 	${CC} ${CFLAGS} ${OPTFLAG} modelica_test.o ${BASE_OBJS} ${INCLUDE}
