#ifndef _customer_h_
#define _customer_h_

#include <memory>
#include "adevs/adevs.h"

/**
 A Busy-Mart customer.
 */
struct Customer {

    /// Time needed for the clerk to process the customer
    double time_wait;

    /// Times the customer entered and left the queue
    double time_enter;
    double time_leave;
};

// TODO: This was originally IO_Type, but I don't really like that name.
// TODO: Are ObjectType, DataType, NetworkType, or ValueType better than EventType?
/// Create an abbreviation for the simulation event objects.
typedef adevs::PortValue<std::shared_ptr<Customer>> EventType;

#endif
