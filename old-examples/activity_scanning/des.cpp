#include "des.h"
#include <list>




void Partition::schedule(std::shared_ptr<Event> event) {
    // If this is not ours it must go to its owner
    if (event->partition().get() != this) {
        other_events.push_back(event);
    }
    // If this is a conditional event then
    // put it into the conditional event list
    else if (event->conditional()) {
        conditional_events.push_back(event);
    }
    // Otherwise put this into our schedule
    else {
        std::list<std::shared_ptr<Event>>::iterator iter = future_events.begin();
        for (; iter != future_events.end(); iter++) {
            if ((*iter)->timestamp() > event->timestamp()) {
                future_events.insert(iter, event);
                return;
            }
        }
        future_events.push_back(event);
    }
}

void Partition::delta_int() {
    if (!imminent_events.empty()) {
        exec(imminent_events);
        mode = Mode::CONDITIONAL;
        imminent_events.clear();
    } else {
        mode = Mode::FUTURE;
    }
}

void Partition::delta_ext(double e, std::list<std::shared_ptr<Event>> const &xb) {
    time_now += e;
    mode = Mode::CONDITIONAL;
    std::list<std::shared_ptr<Event>>::const_iterator iter = xb.begin();
    for (; iter != xb.end(); iter++) {
        if (*iter != NULL && (*iter)->partition().get() == this) {
            schedule(*iter);
        }
    }
}

void Partition::delta_conf(std::list<std::shared_ptr<Event>> const &xb) {
    delta_int();
    delta_ext(0.0, xb);
}

double Partition::ta() {
    if (mode == Mode::CONDITIONAL) {
        return 0.0;
    } else if (future_events.empty()) {
        return adevs_inf<double>();
    } else {
        return future_events.front()->timestamp() - time_now;
    }
}

void Partition::output_func(std::list<std::shared_ptr<Event>> &yb) {
    // Notify others
    for (auto ii : other_events) {
        yb.push_back(ii);
    }
    other_events.clear();
    // Find the set of events that we will execute
    // and gather the input for those events
    if (mode == Mode::FUTURE && !future_events.empty()) {
        time_now = future_events.front()->timestamp();
        do {
            future_events.front()->prep();
            imminent_events.push_back(future_events.front());
            future_events.pop_front();
        } while (!future_events.empty() &&
                 future_events.front()->timestamp() == time_now);
    } else if (mode == Mode::CONDITIONAL) {
        std::list<std::shared_ptr<Event>>::iterator iter = conditional_events.begin();
        while (iter != conditional_events.end()) {
            if ((*iter)->prep()) {
                imminent_events.push_back(*iter);
                iter = conditional_events.erase(iter);
            } else {
                iter++;
            }
        }
    }
    // If we are going to execute an event, inform everyone
    // to scan their conditional event list in the next iteration
    if (!imminent_events.empty() && yb.size() == 0) {
        yb.push_back(NULL);
    }
}
