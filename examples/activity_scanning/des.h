#ifndef des_h_
#define des_h_
#include <list>
#include <vector>
#include "adevs.h"

class Partition;

/**
 * This is an event that acts to change the state
 * of a partition.
 */
class Event
{
	public:
		/**
		 * Create an event to act on a partition. The timestamp
		 * must be inf for a conditional event.
		 */
		Event(Partition* p, double t = adevs_inf<double>()):
			p(p),t(t){}
		virtual ~Event(){}
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
		Partition* partition() { return p; }
		/// Get the timestamp
		double timestamp() const { return t; }
		/// Is this a conditional event?
		bool conditional() const { return t == adevs_inf<double>(); }
	private:
		Partition* p;
		const double t;
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
class Partition:
	public adevs::Atomic<Event*>
{
	public:
		Partition():adevs::Atomic<Event*>(),tNow(0.0),mode('F'){}
		virtual ~Partition();
		/**
		 * <p>Execute the events that are imminent for this partition.
		 * The events passed to this method are all of the events
		 * that are scheduled or eligible for activation at the
		 * current simulation time. If there are multiple events
		 * in the list then the partition may act on them in any
		 * way that is appropriate to the model.</p>
		 * <p>All imminent events are deleted when exec returns.</p>
		 */
		virtual void exec(std::vector<Event*>& imm) = 0;
		/// Get the simulation time
		double now() const { return tNow; }
		/// Schedule an event for this or another partition
		void schedule(Event* ev);
	private:
		std::list<Event*> cel; // Conditional event list
		std::list<Event*> fel; // Future event list
		std::vector<Event*> other, // Events for other partitions
		                    imm; // Imminent events at this partition
		double tNow; // Current simulation time
		char mode; // 'F' or 'C'
	public:
		void delta_int();
		void delta_ext(double e, const adevs::Bag<Event*>& xb);
		void delta_conf(const adevs::Bag<Event*>& xb);
		void output_func(adevs::Bag<Event*>& yb);
		void gc_output(adevs::Bag<Event*>&){}
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
typedef adevs::SimpleDigraph<Event*> World;
typedef adevs::Simulator<Event*> Simulator;

#endif
