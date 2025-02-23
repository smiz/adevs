#ifndef _object_h_
#define _object_h_
#include "adevs/adevs.h"

class object {
  public:
    object() {}
    object(object const &) {}
    ~object() {}
};

typedef std::shared_ptr<object> ObjectPtr;

#endif
