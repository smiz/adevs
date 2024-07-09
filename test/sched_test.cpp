#include "adevs/sched.h"
#include <cassert>
#include <iostream>
using namespace adevs;

class bogus_atomic : public Atomic<char> {
  public:
    bogus_atomic() : Atomic<char>() {}
    void delta_int() {}
    void delta_ext(double, Bag<char> const &) {}
    void delta_conf(Bag<char> const &) {}
    void output_func(Bag<char> &) {}
    void gc_output(Bag<char> &) {}
    double ta() { return 0.0; }
};

void testa() {
    Schedule<char> q;
    bogus_atomic m;
    q.schedule(&m, 0.0);
    q.removeMinimum();
    q.schedule(&m, 0.0);
    q.schedule(&m, 1.0);
    assert(q.minPriority() == 1.0);
}

void test1() {
    int i;
    bogus_atomic m[10];
    Schedule<char> q;
    for (i = 0; i < 10; i++) {
        q.schedule(&(m[i]), (double)i);
        assert(q.minPriority() == 0.0);
        assert(q.getMinimum() == &(m[0]));
    }
    for (i = 0; i < 10; i++) {
        assert(q.minPriority() == (double)i);
        assert(q.getMinimum() == &(m[i]));
        q.removeMinimum();
    }
}

void test2() {
    bogus_atomic m[5];
    Schedule<char> q;
    q.schedule(&(m[0]), 1.0);
    q.schedule(&(m[1]), 10.0);
    q.schedule(&(m[2]), 5.0);
    assert(q.minPriority() == 1.0);
    q.removeMinimum();
    assert(q.minPriority() == 5.0);
    q.schedule(&(m[3]), 3.0);
    q.schedule(&(m[4]), 4.0);
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
    bogus_atomic m[3];
    Schedule<char> q;
    q.schedule(&(m[0]), 5.0);
    q.schedule(&(m[1]), 10.0);
    q.schedule(&(m[2]), 1.0);
    q.schedule(&(m[0]), DBL_MAX);
    assert(q.minPriority() == 1.0);
    q.schedule(&(m[2]), DBL_MAX);
    assert(q.minPriority() == 10.0);
}

void test4() {
    Schedule<char> q;
    assert(q.minPriority() == DBL_MAX);
    assert(q.getMinimum() == NULL);
    bogus_atomic m[200];
    srand(200);
    for (int i = 0; i < 300; i++) {
        q.schedule(&(m[rand() % 200]), (double)rand());
    }
    double tL = q.minPriority();
    while (!q.empty()) {
        assert(q.getMinimum() != NULL);
        q.removeMinimum();
        if (!q.empty()) {
            assert(tL <= q.minPriority());
            tL = q.minPriority();
        }
    }
    assert(q.getMinimum() == NULL);
    assert(q.minPriority() == DBL_MAX);
}

void test5() {
    bogus_atomic m[2];
    Schedule<char> q;
    q.schedule(&(m[0]), 2.0);
    q.schedule(&(m[1]), 3.0);
    q.schedule(&(m[1]), DBL_MAX);
    assert(q.minPriority() == 2.0);
    assert(q.getMinimum() == &(m[0]));
}

void test6() {
    bogus_atomic m[2];
    Schedule<char> q;
    q.schedule(&(m[0]), 1.0);
    q.schedule(&(m[1]), 1.0);
    q.schedule(&(m[0]), DBL_MAX);
    assert(q.getMinimum() == &(m[1]));
    assert(q.minPriority() == 1.0);
}

void test7() {
    bogus_atomic m[2];
    Schedule<char> q;
    q.schedule(&(m[0]), 2.0);
    q.schedule(&(m[1]), 3.0);
    q.schedule(&(m[0]), 4.0);
    q.schedule(&(m[0]), 4.0);
    assert(q.getMinimum() == &(m[1]));
    q.removeMinimum();
    assert(q.getMinimum() == &(m[0]));
    q.schedule(&(m[1]), 1.0);
    assert(q.getMinimum() == &(m[1]));
}

void test8() {
    bogus_atomic m[2000];
    Schedule<char> q;
    for (int i = 0; i < 2000; i++) {
        q.schedule(&(m[i]), (double)i);
        assert(q.getMinimum() == &(m[i]));
        q.schedule(&(m[i]), (double)i);
        assert(q.getMinimum() == &(m[i]));
        q.schedule(&(m[i]), DBL_MAX);
        assert(q.empty());
    }
}

void test9() {
    int i;
    bogus_atomic m[20];
    Schedule<char> q;
    for (i = 0; i < 10; i++) {
        q.schedule(&(m[i]), 1.0);
    }
    assert(q.minPriority() == 1.0);
    for (i = 10; i < 20; i++) {
        q.schedule(&(m[i]), 2.0);
    }
    assert(q.minPriority() == 1.0);
}

class test10visitor : public Schedule<char>::ImminentVisitor {
  public:
    test10visitor(Bag<Atomic<char>*> &imm) : imm(imm) {}
    void visit(Atomic<char>* model) { imm.push_back(model); }

  private:
    Bag<Atomic<char>*> &imm;
};

void test10() {
    int i;
    bogus_atomic m[20];
    Schedule<char> q;
    Bag<Atomic<char>*> imm;
    test10visitor* visitor = new test10visitor(imm);
    q.visitImminent(visitor);
    delete visitor;
    assert(imm.empty());
    for (i = 0; i < 10; i++) {
        q.schedule(&(m[i]), 1.0);
    }
    assert(q.minPriority() == 1.0);
    visitor = new test10visitor(imm);
    q.visitImminent(visitor);
    delete visitor;
    assert(imm.size() == 10);
    for (i = 0; i < 10; i++) {
        assert(imm.find(&m[i]) != imm.end());
    }
    imm.clear();
    for (i = 10; i < 20; i++) {
        q.schedule(&(m[i]), 2.0);
    }
    visitor = new test10visitor(imm);
    q.visitImminent(visitor);
    delete visitor;
    assert(imm.size() == 10);
    for (i = 0; i < 10; i++) {
        assert(imm.find(&m[i]) != imm.end());
    }
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
