#ifndef _object_h_
#define _object_h_

class object {
  public:
    object() {}
    object(object const &) {}
    ~object() {}
};

typedef adevs::PortValue<object*> PortValue;

#endif
