#ifndef CART_MODEL_H
#define CART_MODEL_H
#include "adevs/adevs.h"

#ifndef PI
#define PI 3.1415926535897931
#endif
#define RAD_TO_DEG (180.0 / PI)
// This is the dynamic model of the pendulum. The only input
// is the force applied by the cart's motor and the only output
// is the arm angle measurement.
class CartModel : public adevs::ode_system<double> {
  public:
    CartModel();
    // Arm angle in degrees; zero degrees is straight down
    // and rotation is clockwise.
    double angle(double const* q) const { return RAD_TO_DEG * q[theta]; }
    void init(double* q);
    void der_func(double const* q, double* dq);
    void state_event_func(double const* q, double* z);
    void internal_event(double* q, bool const* state_event);
    void external_event(double* q, double e, adevs::Bag<double> const &xb);
    void confluent_event(double* q, bool const* state_event,
                         adevs::Bag<double> const &xb);
    void output_func(double const* q, bool const* state_event,
                     adevs::Bag<double> &yb);
    double time_event_func(double const*) { return DBL_MAX; }
    void gc_output(adevs::Bag<double> &) {}

  private:
    // State variable indices
    int const x, theta, dx, dtheta, t;
    // Model parameters
    double const armMass, armFric, cartMass, cartFric, armLen, g, mAngle;
    int k;  // Last output level for the quantized sensor
    // Constraint matrix and vector for A [ddx ddtheta]^T = B
    double A[4][4], B[2];
    double F;  // Motor force
};

#endif
