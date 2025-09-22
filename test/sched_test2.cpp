#include "adevs/schedule2.h"

#include <cassert>
#include <iostream>
#include <memory>


using Schedule = adevs::Schedule<char>;
using Atomic = adevs::Atomic;

class bogus_atomic : public Atomic {
  public:
    bogus_atomic(double event) : Atomic(), _event(event) {}
    void delta_int() {}
    void delta_ext(double, std::list<PinValue> const &) {}
    void delta_conf(std::list<PinValue> const &) {}
    void output_func(std::list<PinValue> &) {}
    double ta() { return (double)_event; }
    void set(double event) { _event = event; }

  private:
    double _event;
};


void testa() {
    Schedule<char> q;
    std::shared_ptr<bogus_atomic> m = std::make_shared<bogus_atomic>(0.0);
    q.add(m);
    q.remove_next();
    q.add(m);
    // Models can't be added to the schedule twice. Once they are active, use update().
    m->set(1.0);
    q.update(m);
    assert(q.get_minimum() == 1.0);
}

void test1() {
    unsigned int ii;
    std::shared_ptr<bogus_atomic> m[10];
    for (ii = 0; ii < 10; ii++) {
        m[ii] = std::make_shared<bogus_atomic>(ii);
    }

    Schedule q;
    for (ii = 0; ii < 10; ii++) {
        q.add(m[ii]);
        assert(q.get_minimum() == 0.0);
        assert(q.get_next() == m[0]);
    }
    for (ii = 0; ii < 10; ii++) {
        assert(q.get_minimum() == (double)ii);
        assert(q.get_next() == m[ii]);
        q.remove_next();
    }
}

void test2() {
    std::shared_ptr<bogus_atomic> m0 = std::make_shared<bogus_atomic>(1.0);
    std::shared_ptr<bogus_atomic> m1 = std::make_shared<bogus_atomic>(10.0);
    std::shared_ptr<bogus_atomic> m2 = std::make_shared<bogus_atomic>(5.0);
    std::shared_ptr<bogus_atomic> m3 = std::make_shared<bogus_atomic>(3.0);
    std::shared_ptr<bogus_atomic> m4 = std::make_shared<bogus_atomic>(4.0);

    Schedule q;
    q.add(m0);
    q.add(m1);
    q.add(m2);
    assert(q.get_minimum() == 1.0);
    q.remove_next();
    assert(q.get_minimum() == 5.0);
    q.add(m3);
    q.add(m4);
    assert(q.get_minimum() == 3.0);
    q.remove_next();
    assert(q.get_minimum() == 4.0);
    q.remove_next();
    assert(q.get_minimum() == 5.0);
    q.remove_next();
    assert(q.get_minimum() == 10.0);
    q.remove_next();
}

void test3() {
    std::shared_ptr<bogus_atomic> m0 = std::make_shared<bogus_atomic>(5.0);
    std::shared_ptr<bogus_atomic> m1 = std::make_shared<bogus_atomic>(10.0);
    std::shared_ptr<bogus_atomic> m2 = std::make_shared<bogus_atomic>(1.0);

    Schedule q;
    q.add(m0);
    q.add(m1);
    q.add(m2);
    // Set the model's new time advance and update the schedule
    m0->set(DBL_MAX);
    q.update(m0);
    assert(q.get_minimum() == 1.0);
    m2->set(DBL_MAX);
    q.update(m2);
    assert(q.get_minimum() == 10.0);
}

void test4() {
    Schedule q;
    assert(q.get_minimum() == DBL_MAX);
    assert(q.get_next() == NULL);

    std::shared_ptr<bogus_atomic> m[200];
    srand(200);
    for (int ii = 0; ii < 300; ii++) {
        std::shared_ptr<bogus_atomic> m = std::make_shared<bogus_atomic>((double)rand());
        q.add(m);
    }

    double tL = q.get_minimum();
    while (!q.empty()) {
        assert(q.get_next() != nullptr);
        q.remove_next();
        if (!q.empty()) {
            assert(tL <= q.get_minimum());
            tL = q.get_minimum();
        }
    }
    assert(q.get_next() == nullptr);
    assert(q.get_minimum() == DBL_MAX);
}

void test5() {
    std::shared_ptr<bogus_atomic> m0 = std::make_shared<bogus_atomic>(2.0);
    std::shared_ptr<bogus_atomic> m1 = std::make_shared<bogus_atomic>(3.0);
    Schedule q;

    q.add(m0);
    q.add(m1);
    m1->set(DBL_MAX);
    q.update(m1);
    assert(q.get_minimum() == 2.0);
    assert(q.get_next() == m0);
}

void test6() {
    std::shared_ptr<bogus_atomic> m0 = std::make_shared<bogus_atomic>(1.0);
    std::shared_ptr<bogus_atomic> m1 = std::make_shared<bogus_atomic>(1.0);
    Schedule q;

    q.add(m0);
    q.add(m1);
    m0->set(DBL_MAX);
    q.update(m0);
    assert(q.get_next() == m1);
    assert(q.get_minimum() == 1.0);
}

void test7() {
    std::shared_ptr<bogus_atomic> m0 = std::make_shared<bogus_atomic>(2.0);
    std::shared_ptr<bogus_atomic> m1 = std::make_shared<bogus_atomic>(3.0);
    Schedule q;

    q.add(m0);
    q.add(m1);
    m0->set(4.0);
    q.update(m0);
    m0->set(4.0);
    q.update(m0);
    assert(q.get_next() == m1);
    q.remove_next();
    assert(q.get_next() == m0);
    q.add(m1);
    assert(q.get_next() == m1);
}

void test8() {
    std::shared_ptr<bogus_atomic> m[2000];
    Schedule q;

    for (int ii = 0; ii < 2000; ii++) {
        m[ii] = std::make_shared<bogus_atomic>(ii);
        q.add(m[ii]);
        assert(q.get_next() == m[ii]);
        m[ii]->set(ii * 2);
        q.update(m[ii]);
        assert(q.get_next() == m[ii]);
        q.remove(m[ii]);
        assert(q.empty());
    }
}

void test9() {
    int ii;
    std::shared_ptr<bogus_atomic> m[20];
    Schedule q;

    for (ii = 0; ii < 10; ii++) {
        m[ii] = std::make_shared<bogus_atomic>(1.0);
        q.add(m[ii]);
    }
    assert(q.get_minimum() == 1.0);

    for (ii = 10; ii < 20; ii++) {
        m[ii] = std::make_shared<bogus_atomic>(2.0);
        q.add(m[ii]);
    }
    assert(q.get_minimum() == 1.0);
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
    return 0;
}
