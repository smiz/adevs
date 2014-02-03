#include <iostream>
#include <cassert>
#include <sys/types.h>
#include <unistd.h>
#include "adevs.h"
using namespace std;
using namespace adevs;

void test_triangular()
{
	int bins[101];
	for (int i = 0; i < 101; i++)
		bins[i] = 0;
	double a = 0.0, b = 1.0, c = 0.65;
	double sum = 0.0;
	int count = 100000000;
	rv r(getpid());
	for (int i = 0; i < count; i++)
	{
		double s = r.triangular(a,b,c);
		assert(s >= a && s <= b);
		sum += s;
		bins[int(100.0*s)]++;
	}
	int mode_bin = int(100.0*c)-1;
	for (int i = 0; i < 101; i++)
	{
		if (i != mode_bin)
			assert(bins[i] < bins[mode_bin]);
	}
	assert(fabs((a+b+c)/3.0 - sum/(double)(count)) < 1E-2);
	cout << (sum/(double)(count)) << " ~? " << ((a+b+c)/3.0) << endl;
}

int main()
{
	test_triangular();
	return 0;
}
