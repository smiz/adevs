#include "SimpleAtomic.h"
using namespace adevs;

int SimpleAtomic::atomic_number = 0;
int SimpleAtomic::internal_execs = 0;

SimpleAtomic::SimpleAtomic():
Atomic<SimpleIO>()
{
	number = atomic_number++;
}

void SimpleAtomic::delta_int()
{
	internal_execs++;
}

SimpleAtomic::~SimpleAtomic()
{
	atomic_number--;
}

