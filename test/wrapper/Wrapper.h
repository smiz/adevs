#ifndef _wrapper_h_
#define _wrapper_h_
#include <cassert>
#include "adevs/adevs.h"
#include "events.h"

/**
 * This converts between Internal and External event types.
 */
class Wrapper : public adevs::ModelWrapper<External*, Internal*> {
  public:
    Wrapper(adevs::Atomic<Internal*>* model)
        : adevs::ModelWrapper<External*, Internal*>(model) {}
    void translateInput(list<External*> const &external,
                        list<adevs::Event<Internal*>> &internal) {
        list<External*>::const_iterator iter = external.begin();
        adevs::Event<Internal*> event;
        event.model = getWrappedModel();
        if ((*iter)->speed == STOP) {
            event.value = new Internal(DBL_MAX, (*iter)->value);
        } else if ((*iter)->speed == SLOW) {
            event.value = new Internal(2.0, (*iter)->value);
        } else if ((*iter)->speed == FAST) {
            event.value = new Internal(1.0, (*iter)->value);
        }
        internal.push_back(event);
    }
    void translateOutput(list<adevs::Event<Internal*>> const &internal,
                         list<External*> &external) {
        list<adevs::Event<Internal*>>::const_iterator iter = internal.begin();
        assert((*iter).model == getWrappedModel());
        External* e = new External();
        e->value = (*iter).value->value;
        if ((*iter).value->period == DBL_MAX) {
            e->speed = STOP;
        } else if ((*iter).value->period == 2.0) {
            e->speed = SLOW;
        } else if ((*iter).value->period == 1.0) {
            e->speed = FAST;
        } else {
            assert(false);
        }
        external.push_back(e);
    }
};

#endif
