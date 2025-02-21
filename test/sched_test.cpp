#include "adevs/sched.h"
#include <cassert>
#include <iostream>
#include <algorithm>
using namespace adevs;

class bogus_atomic : public Atomic<char> {
  public:
    bogus_atomic() : Atomic<char>() {}
    void delta_int() {}
    void delta_ext(double, list<PinValue<char>> const &) {}
    void delta_conf(list<PinValue<char>> const &) {}
    void output_func(list<PinValue<char>> &) {}

    double ta() { return 0.0; }
};

void testa() {
    Schedule<char> q;
    std::shared_ptr<Atomic<char>> m(new bogus_atomic);
    q.schedule(m, 0.0);
    q.removeMinimum();
    q.schedule(m, 0.0);
    q.schedule(m, 1.0);
    assert(q.minPriority() == 1.0);
}

void test1() {
    int i;
    std::shared_ptr<Atomic<char>> m[10];
    Schedule<char> q;
    for (i = 0; i < 10; i++) {
        m[i] = std::shared_ptr<Atomic<char>>(new bogus_atomic);
        q.schedule(m[i], (double)i);
        assert(q.minPriority() == 0.0);
        assert(q.getMinimum() == m[0]);
    }
    for (i = 0; i < 10; i++) {
        assert(q.minPriority() == (double)i);
        assert(q.getMinimum() == m[i]);
        q.removeMinimum();
    }
}

void test2() {
    std::shared_ptr<Atomic<char>> m[5];
    for (int i = 0; i < 5; i++) {
        m[i] = std::shared_ptr<Atomic<char>>(new bogus_atomic);
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
}

void test3() {
    std::shared_ptr<Atomic<char>> m[3];
    for (int i = 0; i < 3; i++) {
        m[i] = std::shared_ptr<Atomic<char>>(new bogus_atomic);
    }
    Schedule<char> q;
    q.schedule(m[0], 5.0);
    q.schedule(m[1], 10.0);
    q.schedule(m[2], 1.0);
    q.schedule(m[0], adevs_inf<double>());
    assert(q.minPriority() == 1.0);
    q.schedule(m[2], adevs_inf<double>());
    assert(q.minPriority() == 10.0);
}

void test4() {
    Schedule<char> q;
    assert(q.minPriority() == adevs_inf<double>());
    assert(q.getMinimum() == nullptr);
    std::shared_ptr<Atomic<char>> m[200];
    for (int i = 0; i < 200; i++) {
        m[i] = std::shared_ptr<Atomic<char>>(new bogus_atomic);
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
}

void test5() {
    std::shared_ptr<Atomic<char>> m[2];
    for (int i = 0; i < 2; i++) {
        m[i] = std::shared_ptr<Atomic<char>>(new bogus_atomic);
    }
    Schedule<char> q;
    q.schedule(m[0], 2.0);
    q.schedule(m[1], 3.0);
    q.schedule(m[1], DBL_MAX);
    assert(q.minPriority() == 2.0);
    assert(q.getMinimum() == m[0]);
}

void test6() {
    std::shared_ptr<Atomic<char>> m[2];
    for (int i = 0; i < 2; i++) {
        m[i] = std::shared_ptr<Atomic<char>>(new bogus_atomic);
    }
    Schedule<char> q;
    q.schedule(m[0], 1.0);
    q.schedule(m[1], 1.0);
    q.schedule(m[0], adevs_inf<double>());
    assert(q.getMinimum() == m[1]);
    assert(q.minPriority() == 1.0);
}

void test7() {
    std::shared_ptr<Atomic<char>> m[2];
    for (int i = 0; i < 2; i++) {
        m[i] = std::shared_ptr<Atomic<char>>(new bogus_atomic);
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
}

void test8() {
    std::shared_ptr<Atomic<char>> m[2000];
    Schedule<char> q;
    for (int i = 0; i < 2000; i++) {
        m[i] = std::shared_ptr<Atomic<char>>(new bogus_atomic);
        q.schedule(m[i], (double)i);
        assert(q.getMinimum() == m[i]);
        q.schedule(m[i], (double)i);
        assert(q.getMinimum() == m[i]);
        q.schedule(m[i], adevs_inf<double>());
        assert(q.empty());
    }
}

void test9() {
    int i;
    std::shared_ptr<Atomic<char>> m[20];
    Schedule<char> q;
    for (i = 0; i < 10; i++) {
        m[i] = std::shared_ptr<Atomic<char>>(new bogus_atomic);
        q.schedule(m[i], 1.0);
    }
    assert(q.minPriority() == 1.0);
    for (i = 10; i < 20; i++) {
        m[i] = std::shared_ptr<Atomic<char>>(new bogus_atomic);
        q.schedule(m[i], 2.0);
    }
    assert(q.minPriority() == 1.0);
}

void test10() {
    std::shared_ptr<Atomic<char>> m0(new bogus_atomic);
    std::shared_ptr<Atomic<char>> m1(new bogus_atomic);
    std::shared_ptr<Atomic<char>> m2(new bogus_atomic);
    Schedule<char> q;
    q.schedule(m0, 1.0);
    q.schedule(m1, 1.0);
    q.schedule(m2, 2.0);
    auto imm = q.visitImminent();
    assert(std::find(imm.begin(), imm.end(), m0) != imm.end());
    assert(std::find(imm.begin(), imm.end(), m1) != imm.end());
    assert(std::find(imm.begin(), imm.end(), m2) == imm.end());
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
