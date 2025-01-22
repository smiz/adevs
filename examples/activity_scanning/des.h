#ifndef des_h_
#define des_h_

#include <list>
#include <memory>
#include <vector>

#include "adevs/adevs.h"

using namespace std;


enum class Mode {
    FUTURE = 0,
    CONDITIONAL = 1,
};


class Partition;


/**
 * This is an event that acts to change the state
 * of a partition.
 */
class Event {
  public:
    /**
     * Create an event to act on a partition. The timestamp
     * must be inf for a conditional event.
     */
    Event(shared_ptr<Partition> p, double t = adevs_inf<double>())
        : _partition(p), _time(t) {}
    virtual ~Event() {}
    /**
     * Gather the data that is needed to execute
     * this event. If this is a conditional event,
     * then return true if the event condition is
     * satisfied and false otherwise. The return
     * value is not used if this is not a conditional
     * event.
     */
    virtual bool prep() = 0;
    /// Execute the event.
    virtual void exec() = 0;
    /// Get the partition
    shared_ptr<Partition> partition() { return _partition; }
    /// Get the timestamp
    double timestamp() const { return _time; }
    /// Is this a conditional event?
    bool conditional() const { return _time == adevs_inf<double>(); }

  private:
    shared_ptr<Partition> _partition;
    double const _time;
};

/**
 * <p>A partition is a collection of variables in your model
 * chosen such that any event acting on those variables
 * does not act on variables in any other partition. The
 * events acting on a partition may, during the execution
 * of their prep method, gather data from many partitions.
 * However, when executed that event must only change variables
 * within the partition.</p>
 */
class Partition : public adevs::Atomic<shared_ptr<Event>> {
  public:
    Partition()
        : adevs::Atomic<shared_ptr<Event>>(),
          time_now(0.0),
          mode(Mode::FUTURE) {}

    /**
     * <p>Execute the events that are imminent for this partition.
     * The events passed to this method are all of the events
     * that are scheduled or eligible for activation at the
     * current simulation time. If there are multiple events
     * in the list then the partition may act on them in any
     * way that is appropriate to the model.</p>
     * <p>All imminent events are deleted when exec returns.</p>
     */
    virtual void exec(std::vector<shared_ptr<Event>> &imminent) = 0;
    /// Get the simulation time
    double now() const { return time_now; }
    /// Schedule an event for this or another partition
    void schedule(shared_ptr<Event> event);

  private:
    std::list<shared_ptr<Event>> conditional_events;
    std::list<shared_ptr<Event>> future_events;
    std::vector<shared_ptr<Event>> other_events;
    std::vector<shared_ptr<Event>> imminent_events;
    double time_now;
    Mode mode;

  public:
    void delta_int();
    void delta_ext(double e, list<shared_ptr<Event>> const &xb);
    void delta_conf(list<shared_ptr<Event>> const &xb);
    void output_func(list<shared_ptr<Event>> &yb);
    double ta();
};

/**
 * To create a model you must put your partitions into
 * a World object and if a partition A requires data
 * from or has events scheduled for it by a partition B
 * the couple B to A so that B will notify A of its
 * activities. The World class is an instance of an
 * adevs::SimpleDigraph object.
 */
typedef adevs::SimpleDigraph<shared_ptr<Event>> World;
typedef adevs::Simulator<shared_ptr<Event>> Simulator;

#endif
