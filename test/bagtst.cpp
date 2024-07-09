#include <cassert>
#include "adevs.h"
using namespace adevs;

template <class X>
class template_test {
  public:
    X* to_array(Bag<X> const &b) {
        int i = 0;
        X* array = new X[b.size()];
        typename Bag<X>::const_iterator iter = b.begin();
        for (; iter != b.end(); iter++) {
            array[i] = *iter;
            i++;
        }
        return array;
    }
};

/**
 * This is to test the use of template iterators inside of
 * a template class. The main purpose of the test is to
 * make sure the template instantiation works as intended.
 */
void test1() {
    Bag<int> b;
    b.insert(0);
    template_test<int> t;
    int* array = t.to_array(b);
    assert(array[0] == 0);
}

/**
 * Test the swap method.
 */
void test2() {
    Bag<int> a, b;
    for (int i = 0; i < 10; i++) {
        b.insert(i);
    }
    a.swap(b);
    assert(a.size() == 10);
    assert(b.size() == 0);
    int k = 0;
    for (Bag<int>::iterator i = a.begin(); i != a.end(); i++) {
        assert(*i == k);
        k++;
    }
}

int main() {
    test1();
    test2();
    return 0;
}
