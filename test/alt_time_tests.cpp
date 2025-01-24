/**
 * Test cases for alternate types of time.
 */

#include <memory>

#include "adevs/adevs.h"

using namespace std;
using namespace adevs;


// ***** Basic model ******

template <typename TimeType>
class PingPong : public Atomic<int, TimeType> {
  public:
    PingPong(bool active = false);
    void delta_int();
    void delta_ext(TimeType e, list<int> const &xb);
    void delta_conf(list<int> const &xb);
    void output_func(list<int> &yb);
    TimeType ta();
    int getCount() const { return count; }

  private:
    int count;
    bool active;
};

template <typename TimeType>
PingPong<TimeType>::PingPong(bool active)
    : Atomic<int, TimeType>(), count(0), active(active) {}

template <typename TimeType>
void PingPong<TimeType>::delta_int() {
    count++;
    active = false;
}

template <typename TimeType>
void PingPong<TimeType>::delta_ext(TimeType e, list<int> const &xb) {
    active = xb.size() == 1;
}

template <typename TimeType>
void PingPong<TimeType>::delta_conf(list<int> const &xb) {
    delta_int();
    delta_ext(0, xb);
}

template <typename TimeType>
TimeType PingPong<TimeType>::ta() {
    if (active) {
        return 1;
    } else {
        return adevs_inf<TimeType>();
    }
}

template <typename TimeType>
void PingPong<TimeType>::output_func(list<int> &yb) {
    yb.push_back(1);
}


// ***** Basic network *****

template <typename TimeType>
class Model : public SimpleDigraph<int, TimeType> {
  public:
    Model();
    shared_ptr<PingPong<TimeType>> getA() { return a; }
    shared_ptr<PingPong<TimeType>> getB() { return b; }

  private:
    shared_ptr<PingPong<TimeType>> a = nullptr;
    shared_ptr<PingPong<TimeType>> b = nullptr;
};

template <typename TimeType>
Model<TimeType>::Model()
    : SimpleDigraph<int, TimeType>(),
      a(make_shared<PingPong<TimeType>>(true)),
      b(make_shared<PingPong<TimeType>>()) {
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
