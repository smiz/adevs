#include "adevs/sched.h"
#include <algorithm>
#include <cassert>
#include <iostream>
using namespace adevs;

class bogus_atomic : public Atomic<char> {
  public:
    bogus_atomic() : Atomic<char>() {}
    void delta_int() {}
    void delta_ext(double, std::list<PinValue<char>> const &) {}
    void delta_conf(std::list<PinValue<char>> const &) {}
    void output_func(std::list<PinValue<char>> &) {}
    double ta() { return 0.0; }
};

void testa() {
    Schedule<char> q;
    Atomic<char>* m = new bogus_atomic;
    q.schedule(m, 0.0);
    q.removeMinimum();
    q.schedule(m, 0.0);
    q.schedule(m, 1.0);
    assert(q.minPriority() == 1.0);
    delete m;
}

void test1() {
    int i;
    Atomic<char>* m[10];
    Schedule<char> q;
    for (i = 0; i < 10; i++) {
        m[i] = new bogus_atomic;
        q.schedule(m[i], (double)i);
        assert(q.minPriority() == 0.0);
        assert(q.getMinimum() == m[0]);
    }
    for (i = 0; i < 10; i++) {
        assert(q.minPriority() == (double)i);
        assert(q.getMinimum() == m[i]);
        q.removeMinimum();
    }
    for (i = 0; i < 10; i++) {
        delete m[i];
    }
}

void test2() {
    Atomic<char>* m[5];
    for (int i = 0; i < 5; i++) {
        m[i] = new bogus_atomic;
    }
    Schedule<char> q;
    q.schedule(m[0], 1.0);
    q.schedule(m[1], 10.0);
    q.schedule(m[2], 5.0);
    assert(q.minPriority() == 1.0);
    q.removeMinimum();
    assert(q.minPriority() == 5.0);
    q.schedule(m[3], 3.0);
    q.schedule(m[4], 4.0);
    assert(q.minPriority() == 3.0);
    q.removeMinimum();
    assert(q.minPriority() == 4.0);
    q.removeMinimum();
    assert(q.minPriority() == 5.0);
    q.removeMinimum();
    assert(q.minPriority() == 10.0);
    q.removeMinimum();
    for (int i = 0; i < 5; i++) {
        delete m[i];
    }
}

void test3() {
    Atomic<char>* m[3];
    for (int i = 0; i < 3; i++) {
        m[i] = new bogus_atomic;
    }
    Schedule<char> q;
    q.schedule(m[0], 5.0);
    q.schedule(m[1], 10.0);
    q.schedule(m[2], 1.0);
    q.schedule(m[0], adevs_inf<double>());
    assert(q.minPriority() == 1.0);
    q.schedule(m[2], adevs_inf<double>());
    assert(q.minPriority() == 10.0);
    for (int i = 0; i < 3; i++) {
        delete m[i];
    }
}

void test4() {
    Schedule<char> q;
    assert(q.minPriority() == adevs_inf<double>());
    assert(q.getMinimum() == nullptr);
    Atomic<char>* m[200];
    for (int i = 0; i < 200; i++) {
        m[i] = new bogus_atomic;
    }
    srand(200);
    for (int i = 0; i < 300; i++) {
        q.schedule(m[rand() % 200], (double)rand());
    }
    double tL = q.minPriority();
    while (!q.empty()) {
        assert(q.getMinimum() != nullptr);
        q.removeMinimum();
        if (!q.empty()) {
            assert(tL <= q.minPriority());
            tL = q.minPriority();
        }
    }
    assert(q.getMinimum() == nullptr);
    assert(q.minPriority() == adevs_inf<double>());
    for (int i = 0; i < 200; i++) {
        delete m[i];
    }
}

void test5() {
    Atomic<char>* m[2];
    for (int i = 0; i < 2; i++) {
        m[i] = new bogus_atomic;
    }
    Schedule<char> q;
    q.schedule(m[0], 2.0);
    q.schedule(m[1], 3.0);
    q.schedule(m[1], DBL_MAX);
    assert(q.minPriority() == 2.0);
    assert(q.getMinimum() == m[0]);
    for (int i = 0; i < 2; i++) {
        delete m[i];
    }
}

void test6() {
    Atomic<char>* m[2];
    for (int i = 0; i < 2; i++) {
        m[i] = new bogus_atomic;
    }
    Schedule<char> q;
    q.schedule(m[0], 1.0);
    q.schedule(m[1], 1.0);
    q.schedule(m[0], adevs_inf<double>());
    assert(q.getMinimum() == m[1]);
    assert(q.minPriority() == 1.0);
    for (int i = 0; i < 2; i++) {
        delete m[i];
    }
}

void test7() {
    Atomic<char>* m[2];
    for (int i = 0; i < 2; i++) {
        m[i] = new bogus_atomic;
    }
    Schedule<char> q;
    q.schedule(m[0], 2.0);
    q.schedule(m[1], 3.0);
    q.schedule(m[0], 4.0);
    q.schedule(m[0], 4.0);
    assert(q.getMinimum() == m[1]);
    q.removeMinimum();
    assert(q.getMinimum() == m[0]);
    q.schedule(m[1], 1.0);
    assert(q.getMinimum() == m[1]);
    for (int i = 0; i < 2; i++) {
        delete m[i];
    }
}

void test8() {
    Atomic<char>* m[2000];
    Schedule<char> q;
    for (int i = 0; i < 2000; i++) {
        m[i] = new bogus_atomic;
        q.schedule(m[i], (double)i);
        assert(q.getMinimum() == m[i]);
        q.schedule(m[i], (double)i);
        assert(q.getMinimum() == m[i]);
        q.schedule(m[i], adevs_inf<double>());
        assert(q.empty());
    }
    for (int i = 0; i < 2000; i++) {
        delete m[i];
    }
}

void test9() {
    int i;
    Atomic<char>* m[20];
    Schedule<char> q;
    for (i = 0; i < 10; i++) {
        m[i] = new bogus_atomic;
        q.schedule(m[i], 1.0);
    }
    assert(q.minPriority() == 1.0);
    for (i = 10; i < 20; i++) {
        m[i] = new bogus_atomic;
        q.schedule(m[i], 2.0);
    }
    assert(q.minPriority() == 1.0);
    for (i = 0; i < 20; i++) {
        delete m[i];
    }
}

void test10() {
    Atomic<char>* m0 = new bogus_atomic;
    Atomic<char>* m1 = new bogus_atomic;
    Atomic<char>* m2 = new bogus_atomic;
    Schedule<char> q;
    q.schedule(m0, 1.0);
    q.schedule(m1, 1.0);
    q.schedule(m2, 2.0);
    auto imm = q.visitImminent();
    assert(std::find(imm.begin(), imm.end(), m0) != imm.end());
    assert(std::find(imm.begin(), imm.end(), m1) != imm.end());
    assert(std::find(imm.begin(), imm.end(), m2) == imm.end());
    delete m0;
    delete m1;
    delete m2;
}

int main() {
    testa();
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
    test9();
    test10();
    return 0;
}
