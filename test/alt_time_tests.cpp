/**
 * Test cases for alternate types of time.
 */

#include <memory>

#include "adevs/adevs.h"


// using namespace adevs;


// ***** Basic model ******

template <typename TimeType>
class PingPong : public adevs::Atomic<int, TimeType> {
  public:
	adevs::pin_t output_pin;
    PingPong(bool active = false);
    void delta_int();
    void delta_ext(TimeType e, std::list<adevs::PinValue<int>> const &xb);
    void delta_conf(std::list<adevs::PinValue<int>> const &xb);
    void output_func(std::list<adevs::PinValue<int>> &yb);
    TimeType ta();
    int getCount() const { return count; }

  private:
    int count;
    bool active;
};

template <typename TimeType>
PingPong<TimeType>::PingPong(bool active)
    : adevs::Atomic<int, TimeType>(), count(0), active(active) {}

template <typename TimeType>
void PingPong<TimeType>::delta_int() {
    count++;
    active = false;
}

template <typename TimeType>
void PingPong<TimeType>::delta_ext(TimeType, std::list<adevs::PinValue<int>> const &xb) {
    active = xb.size() == 1;
}

template <typename TimeType>
void PingPong<TimeType>::delta_conf(std::list<adevs::PinValue<int>> const &xb) {
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
void PingPong<TimeType>::output_func(std::list<adevs::PinValue<int>> &yb) {
	adevs::PinValue y(output_pin, 1);
    yb.push_back(y);
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
    CustomTimeType(int init) : time(init) {}

  private:
    int time;

    friend CustomTimeType adevs_inf<CustomTimeType>();
    friend CustomTimeType adevs_zero<CustomTimeType>();
    friend CustomTimeType adevs_sentinel<CustomTimeType>();
    friend CustomTimeType adevs_epsilon<CustomTimeType>();
    friend class PingPong<CustomTimeType>;
    friend void test3();
};

template <>
inline CustomTimeType adevs_inf<CustomTimeType>() {
    return CustomTimeType(std::numeric_limits<int>::max());
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

template <typename TimeType>
class Model : public adevs::Graph<int, TimeType> {
  public:
    Model() : adevs::Graph<int, TimeType>() {
    	adevs::pin_t pA, pB;
        A = std::shared_ptr<PingPong<TimeType>>(new PingPong<TimeType>(true));
        B = std::make_shared<PingPong<TimeType>>();
        this->add_atomic(A);
        this->add_atomic(B);
        this->connect(pA, A);
        this->connect(pB, B);
        A->output_pin = pB;
        B->output_pin = pA;
    }
    PingPong<TimeType>* getA() { return A.get(); }
    PingPong<TimeType>* getB() { return B.get(); }

  private:
    std::shared_ptr<PingPong<TimeType>> A;
    std::shared_ptr<PingPong<TimeType>> B;
};


// ***** Tests *****

void test1() {
    auto model = std::make_shared<Model<int>>();
    std::shared_ptr<adevs::Simulator<int, int>> sim =
        std::make_shared<adevs::Simulator<int, int>>(model);
    while (sim->nextEventTime() <= 10) {
        sim->execNextEvent();
    }
    assert(model->getA()->getCount() == 5);
    assert(model->getB()->getCount() == 5);
}

void test2() {
    auto model = std::make_shared<Model<CustomTimeType>>();
    std::shared_ptr<adevs::Simulator<int, CustomTimeType>> sim =
        std::make_shared<adevs::Simulator<int, CustomTimeType>>(model);
    while (sim->nextEventTime() <= CustomTimeType(10)) {
        sim->execNextEvent();
    }
    assert(model->getA()->getCount() == 5);
    assert(model->getB()->getCount() == 5);
}

int main() {
    test1();
    test2();
}
