/**
 * Test cases for alternate types of time.
 */
#include <memory>

#include "adevs/adevs.h"

using namespace std;
using namespace adevs;


// ***** Basic model ******

template <typename T>
class PingPong : public Atomic<int, T> {
  public:
    PingPong(bool active = false);
    void delta_int();
    void delta_ext(T e, list<int> const &xb);
    void delta_conf(list<int> const &xb);
    void output_func(list<int> &yb);
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
void PingPong<T>::delta_ext(T e, list<int> const &xb) {
    active = xb.size() == 1;
}

template <typename T>
void PingPong<T>::delta_conf(list<int> const &xb) {
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
void PingPong<T>::output_func(list<int> &yb) {
    yb.push_back(1);
}


// ***** Basic network *****

template <typename T>
class Model : public SimpleDigraph<int, T> {
  public:
    Model();
    shared_ptr<PingPong<T>> getA() { return a; }
    shared_ptr<PingPong<T>> getB() { return b; }

  private:
    shared_ptr<PingPong<T>> a = nullptr;
    shared_ptr<PingPong<T>> b = nullptr;
};

template <typename T>
Model<T>::Model()
    : SimpleDigraph<int, T>(),
      a(make_shared<PingPong<T>>(true)),
      b(make_shared<PingPong<T>>()) {
    this->add(a);
    this->add(b);
    this->couple(a, b);
    this->couple(b, a);
}


// ***** Custom Time Datatype *****

// Non-standard type for time
class CustomTimeType {
  public:
    CustomTimeType() : time(0) {}

    CustomTimeType(CustomTimeType const &src) : time(src.time) {}

    // *** Modification Operators ***
    CustomTimeType &operator=(CustomTimeType const &src) {
        time = src.time;
        return *this;
    }

    CustomTimeType operator+(CustomTimeType const &other) const {
        return CustomTimeType(time + other.time);
    }

    CustomTimeType operator-(CustomTimeType const &other) const {
        return CustomTimeType(time - other.time);
    }

    CustomTimeType &operator+=(CustomTimeType const &other) {
        time += other.time;
        return *this;
    }

    // *** Comparison Operators ***

    bool operator<(CustomTimeType const &other) const {
        return time < other.time;
    }

    bool operator==(CustomTimeType const &other) const {
        return time == other.time;
    }

    bool operator<=(CustomTimeType const &other) const {
        return time == other.time || time < other.time;
    }

    bool operator>(CustomTimeType const &other) const {
        return time > other.time;
    }

    bool operator>=(CustomTimeType const &other) const {
        return time >= other.time;
    }

  private:
    int time;
    CustomTimeType(int init) : time(init) {}

    friend CustomTimeType adevs_inf<CustomTimeType>();
    friend CustomTimeType adevs_zero<CustomTimeType>();
    friend CustomTimeType adevs_sentinel<CustomTimeType>();
    friend CustomTimeType adevs_epsilon<CustomTimeType>();
    friend class PingPong<CustomTimeType>;
    friend void test3();
};

template <>
inline CustomTimeType adevs_inf<CustomTimeType>() {
    return CustomTimeType(numeric_limits<int>::max());
}

template <>
inline CustomTimeType adevs_zero<CustomTimeType>() {
    return CustomTimeType(0);
}

template <>
inline CustomTimeType adevs_epsilon<CustomTimeType>() {
    return CustomTimeType(0);
}

template <>
inline CustomTimeType adevs_sentinel<CustomTimeType>() {
    return CustomTimeType(-1);
}


// ***** Tests *****

void test1() {
    shared_ptr<Model<double>> model = make_shared<Model<double>>();
    shared_ptr<Simulator<int, double>> sim =
        make_shared<Simulator<int, double>>(model);

    while (sim->nextEventTime() <= 10) {
        sim->execNextEvent();
    }

    assert(model->getA()->getCount() == 5);
    assert(model->getB()->getCount() == 5);
}

void test2() {
    shared_ptr<Model<int>> model = make_shared<Model<int>>();
    shared_ptr<Simulator<int, int>> sim =
        make_shared<Simulator<int, int>>(model);

    while (sim->nextEventTime() <= 10) {
        sim->execNextEvent();
    }

    assert(model->getA()->getCount() == 5);
    assert(model->getB()->getCount() == 5);
}

void test3() {
    shared_ptr<Model<CustomTimeType>> model =
        make_shared<Model<CustomTimeType>>();
    shared_ptr<Simulator<int, CustomTimeType>> sim =
        make_shared<Simulator<int, CustomTimeType>>(model);

    while (sim->nextEventTime().time <= 10) {
        sim->execNextEvent();
    }

    assert(model->getA()->getCount() == 5);
    assert(model->getB()->getCount() == 5);
}

int main() {
    test1();
    test2();
    test3();
}
