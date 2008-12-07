#include "adevs_list.h"
#include <cassert>
using namespace adevs;

#define COUNT 100
class item: public ulist<item>::one_list{};
item* items[COUNT];

void make_items()
{
	for (int i = 0; i < COUNT; i++)
		items[i] = new item;
}

void unmake_items()
{
	for (int i = 0; i < COUNT; i++)
		delete items[i];
}

void test1()
{
	make_items();
	ulist<item> l;
	for (int i = 0; i < COUNT; i++)
	{
		l.push_back(items[i]);
		assert(l.back() == items[i]);
		assert(!l.empty());
	}
	unmake_items();
}

void test2()
{
	make_items();
	ulist<item> l;
	for (int i = 0; i < COUNT; i++)
	{
		l.push_front(items[i]);
		assert(l.front() == items[i]);
		assert(!l.empty());
	}
	unmake_items();
}

void test3()
{
	make_items();
	ulist<item> l;
	l.push_front(items[0]);
	l.push_back(items[1]);
	ulist<item>::iterator iter = l.begin();
	iter++;
	l.insert(iter,items[COUNT-1]);
	iter = l.begin();
	assert((*iter) == items[0]);
	iter++;
	assert((*iter) == items[COUNT-1]);
	iter++;
	assert((*iter) == items[1]);
	iter++;
	assert(iter == l.end());
	assert(l.front() == items[0]);
	assert(l.back() == items[1]);
	unmake_items();
}

void test4()
{
	make_items();
	ulist<item> l;
	int i;
	for (i = 0; i < COUNT; i++)
		l.push_back(items[i]);
	ulist<item>::iterator iter = l.begin();
	i = 0;
	while (iter != l.end())
	{
		assert((*iter) == items[i]);
		if (i%2 == 0) iter = l.erase(iter);
		else iter++;
		i++;
	}
	i = 1;
	for (iter = l.begin(); iter != l.end(); iter++)
	{
		assert((*iter) == items[i]);
		i += 2;
	}
	unmake_items();
}

void test5()
{
	make_items();
	ulist<item> l[2];
	int i;
	for (i = 0; i < COUNT; i++)
		l[0].push_back(items[i]);
	ulist<item>::iterator iter = l[0].begin();
	i = 0;
	while (iter != l[0].end())
	{
		assert((*iter) == items[i]);
		if (i%2 == 0) iter = l[0].erase(iter,&l[1]);
		else iter++;
		i++;
	}
	assert(l[1].empty() == false);
	i = 1;
	for (iter = l[0].begin(); iter != l[0].end(); iter++)
	{
		assert((*iter) == items[i]);
		i += 2;
	}
	i = 0;
	for (iter = l[1].begin(); iter != l[1].end(); iter++)
	{
		assert((*iter) == items[i]);
		i += 2;
	}
	for (i = 0; i < 2; i++)
	{
		iter = l[i].begin();
		while (iter != l[i].end())
			iter = l[i].erase(iter);
		assert(l[i].empty());
		assert(l[i].back() == NULL);
	}
	unmake_items();
}

int main()
{
	test1();
	test2();
	test3();
	test4();
	test5();
	return 0;
}
