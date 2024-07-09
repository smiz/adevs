#include "adevs.h"
using namespace std;
using namespace adevs;

/**
 * Test cases for alternate types of time.
 */
template <typename T>
class PingPong : public Atomic<int, T> {
  public:
    PingPong(bool active = false);
    void delta_int();
    void delta_ext(T e, Bag<int> const &xb);
    void delta_conf(Bag<int> const &xb);
    void output_func(Bag<int> &yb);
    void gc_output(Bag<int> &) {}
    T ta();
    int getCount() const { return count; }

  private:
    int count;
    bool active;
};

template <typename T>
PingPong<T>::PingPong(bool active)
    : Atomic<int, T>(), count(0), active(active) {}

template <typename T>
void PingPong<T>::delta_int() {
    count++;
    active = false;
}

template <typename T>
void PingPong<T>::delta_ext(T e, Bag<int> const &xb) {
    active = xb.size() == 1;
}

template <typename T>
void PingPong<T>::delta_conf(Bag<int> const &xb) {
    delta_int();
    delta_ext(0, xb);
}

template <typename T>
T PingPong<T>::ta() {
    if (active) {
        return 1;
    } else {
        return adevs_inf<T>();
    }
}

template <typename T>
void PingPong<T>::output_func(Bag<int> &yb) {
    yb.insert(1);
}

template <typename T>
class Model : public SimpleDigraph<int, T> {
  public:
    Model();
    PingPong<T>* getA() { return a; }
    PingPong<T>* getB() { return b; }

  private:
    PingPong<T>*a, *b;
};

template <typename T>
Model<T>::Model()
    : SimpleDigraph<int, T>(), a(new PingPong<T>(true)), b(new PingPong<T>()) {
    this->add(a);
    this->add(b);
    this->couple(a, b);
    this->couple(b, a);
}
// Non-standard type for time
class TimeType {
  public:
    TimeType() : t(0) {}
    TimeType(TimeType const &src) : t(src.t) {}
    TimeType &operator=(TimeType const &src) {
        t = src.t;
        return *this;
    }
    TimeType operator+(TimeType const &b) const { return TimeType(t + b.t); }
    TimeType operator-(TimeType const &b) const { return TimeType(t - b.t); }
    TimeType &operator+=(TimeType const &b) {
        t += b.t;
        return *this;
    }
    bool operator<(TimeType const &b) const { return t < b.t; }
    bool operator==(TimeType const &b) const { return t == b.t; }
    bool operator<=(TimeType const &b) const { return t == b.t || t < b.t; }
    bool operator>(TimeType const &b) const { return t > b.t; }
    bool operator>=(TimeType const &b) const { return t >= b.t; }

  private:
    int t;
    TimeType(int init) : t(init) {}

    friend TimeType adevs_inf<TimeType>();
    friend TimeType adevs_zero<TimeType>();
    friend TimeType adevs_sentinel<TimeType>();
    friend TimeType adevs_epsilon<TimeType>();
    friend class PingPong<TimeType>;
    friend void test3();
};

template <>
inline TimeType adevs_inf<TimeType>() {
    return TimeType(numeric_limits<int>::max());
}

template <>
inline TimeType adevs_zero<TimeType>() {
    return TimeType(0);
}

template <>
inline TimeType adevs_epsilon<TimeType>() {
    return TimeType(0);
}

template <>
inline TimeType adevs_sentinel<TimeType>() {
    return TimeType(-1);
}

void test1() {
    Model<double>* model = new Model<double>();
    Simulator<int, double>* sim = new Simulator<int, double>(model);
    while (sim->nextEventTime() <= 10) {
        sim->execNextEvent();
    }
    assert(model->getA()->getCount() == 5);
    assert(model->getB()->getCount() == 5);
    delete sim;
}

void test2() {
    Model<int>* model = new Model<int>();
    Simulator<int, int>* sim = new Simulator<int, int>(model);
    while (sim->nextEventTime() <= 10) {
        sim->execNextEvent();
    }
    assert(model->getA()->getCount() == 5);
    assert(model->getB()->getCount() == 5);
    delete sim;
}

void test3() {
    Model<TimeType>* model = new Model<TimeType>();
    Simulator<int, TimeType>* sim = new Simulator<int, TimeType>(model);
    while (sim->nextEventTime().t <= 10) {
        sim->execNextEvent();
    }
    assert(model->getA()->getCount() == 5);
    assert(model->getB()->getCount() == 5);
    delete sim;
}

int main() {
    test1();
    test2();
    test3();
}
