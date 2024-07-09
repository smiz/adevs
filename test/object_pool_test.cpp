#include "object_pool.h"
#include <cassert>
using namespace adevs;

struct test_struct {
    char c;
};

void test1() {
    int i;
    object_pool<test_struct> pool;
    test_struct* objs[100];
    for (i = 0; i < 100; i++) {
        objs[i] = pool.make_obj();
        assert(objs[i] != NULL);
    }
    for (i = 0; i < 100; i++) {
        pool.destroy_obj(objs[i]);
        objs[i] = NULL;
    }
    for (i = 0; i < 100; i++) {
        objs[i] = pool.make_obj();
        assert(objs[i] != NULL);
    }
    for (i = 0; i < 100; i++) {
        pool.destroy_obj(objs[i]);
        objs[i] = NULL;
    }
}

int main() {
    test1();
    return 0;
}
