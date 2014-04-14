#include <iostream>
#include "adevs_public_modelica_runtime.h"
#include <cassert>
using namespace std;

void test1()
{
	modelica_array<int> A;
	A.push_dim(2);
	assert(A.dim_size(0)==2);
	A.push_dim(2);
	assert(A.dim_size(1)==2);
	A.done_dims();
	A.get(0,0)=0;
	A.get(1,0)=1;
	A.get(0,1)=2;
	A.get(1,1)=3;
	assert(A.get(0,0)==0);
	assert(A.get(1,0)==1);
	assert(A.get(0,1)==2);
	assert(A.get(1,1)==3);
}

void test2()
{
	modelica_array<int> A;
	A.push_dim(10);
	A.done_dims();
	for (int i = 0; i < 10; i++)
		A.get(i) = i;
	for (int i = 0; i < 10; i++)
		assert(A.get(i) == i);
}

void test3()
{
	modelica_array<int> A;
	A.push_dim(10);
	A.push_dim(3);
	A.push_dim(5);
	A.done_dims();
	int count = 0;
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 3; j++)
			for (int k = 0; k < 5; k++)
				A.get(i,j,k) = count++;
	count = 0;
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 3; j++)
			for (int k = 0; k < 5; k++)
				assert(A.get(i,j,k) == count++);
}

void test4()
{
	modelica_array<int> A;
	A.push_dim(10);
	A.push_dim(3);
	A.push_dim(5);
	A.push_dim(8);
	A.done_dims();
	int count = 0;
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 3; j++)
			for (int k = 0; k < 5; k++)
				for (int l = 0; l < 8; l++)
					A.get(i,j,k,l) = count++;
	count = 0;
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 3; j++)
			for (int k = 0; k < 5; k++)
				for (int l = 0; l < 8; l++)
					assert(A.get(i,j,k,l) == count++);
}

int main()
{
	test1();
	test2();
	test3();
	test4();
	return 0;
}

