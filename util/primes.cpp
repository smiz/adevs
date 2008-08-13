#include <stdlib.h>
#include <iostream>
#include <assert.h>

// Generate a prime number table where each number is at least
// k times as large as the previous.  Generates N such numbers.

int next_prime(int i, int m) 
{
	int test_int = i;
	int p = 3;
	while (p < m*i)
	{
		int j = 3;
		while (j != test_int)
		{
			if (div(test_int,j).rem == 0) break;
			j += 2;
		}
		if (j == test_int) 
		{
			p = test_int;
		}
		test_int += 2;
	}
	return p;
}

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		cout << argv[0] << " <starting prime> <multiple> <table size>" << endl;
		return 1;
	}
	int p = atoi(argv[1]);
	int m = atoi(argv[2]);
	int N = atoi(argv[3]);
	cout << "/* Table of " << N << " primes each at least " 
		<< m << " times the previous*/" << endl;
	cout << "static const int table_size = " << N << ";" << endl;
	cout << "static const primes[] = {\n\t"<< p;
	for (int n = 0; n < N; n++)
	{
		assert (p > 0);
		p = next_prime(p,m);
		cout << ",\n\t" << p;
		cout.flush();
	}
	cout << "\n}; " << endl;
	return 0;
}

