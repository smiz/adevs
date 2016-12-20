#include <iostream>
#include <cassert>
#include "adevs.h"
using namespace std;
using namespace adevs;

void test_uniform()
{
	int bins[101];
	for (int i = 0; i < 101; i++)
		bins[i] = 0;
	double a = 0.0, b = 1.0;
	double sum = 0.0;
	int count = 100000000;
	rv r((unsigned long)0);
	for (int i = 0; i < count; i++)
	{
		double s = r.uniform(a,b);
		assert(s >= a && s <= b);
		sum += s;
		bins[int(100.0*s)]++;
	}
	for (int i = 1; i < 100; i++)
	{
		int diff = abs(bins[i] - bins[i-1]);
		if (diff >= 10000) cout << i << " " << diff << " " << 
			((double)(diff)/(double)(count)) << endl;
		assert(diff < 10000);
	}
	assert(fabs((a+b)/2.0 - sum/(double)(count)) < 1E-2);
}

void test_triangular()
{
	int bins[101];
	for (int i = 0; i < 101; i++)
		bins[i] = 0;
	double a = 0.0, b = 1.0, c = 0.65;
	double sum = 0.0;
	int count = 100000000;
	rv r((unsigned long)0);
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
	test_uniform();
	return 0;
}
