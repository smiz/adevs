/**
 * This program simulates the inverter and control logic described in
 * Jun Chai and Ricardo Sanfelice, A Robust Hybrid Control Algorithm
 * for a Single-Phase DC/AC Inverter with Variable Input Voltage,
 * Technical Report No. UA/AME/HDC-2013-001, Hybrid Dynamics and Control
 * Laboratory, Department of Aerospace and Mechanical Engineering,
 * University of Arizona, Tucson, Arizona. Unpublished.
 * http://www.u.arizona.edu/!sricardo/index.php?n=Main.TechnicalReports
 */
#include <iostream>
#include "adevs/adevs.h"

using namespace adevs;

#define VOLTAGE 0
#define CURRENT 1

/**
 * Initial conditions that may be set from the command line.
 */
double i0 = 0.1;
double v0 = 0.009;
int q0 = 1;

/**
 * PI
 */
double const pi = 3.1415926535897931;
typedef int IO_Type;

/**
 * ODEs, state event functions, and parameters for the
 * inverter and its control.
 */
class inverter : public ode_system<IO_Type> {
  public:
    inverter()
        : ode_system<IO_Type>(2, 4),  // 2 cont. vars and 4 state events
          R(0.6),                     // Resistor in Ohms
          L(0.1),                     // Inductor in H
          C(0.04),                    // Capacitor in C
          Vdc(5.0),                   // DC Voltage in V
          c(1.0),                     // Center of reference band
          ci(0.99 * c),               // Inner limit of reference band
          co(1.1 * c),                // Outer limit of reference band
          w(2.0 * 50.0 * pi),         // Reference frequency
          a(0.15),                    // Shape of ref. band
          b(a / (C * w)),             // Shape of ref. band
          eps(1E-5)                   // Hysteresis on i
    {}
    // Initialize the state variables
    void init(double* q) {
        qc = q0;          // Initial control selection
        q[VOLTAGE] = v0;  // Initial voltage
        q[CURRENT] = i0;  // Initial current
        qc = control(q[VOLTAGE], q[CURRENT],
                     qc);  // Calculate the initial control
    }
    // Calculate the derivative functions for i and v
    void der_func(double const* q, double* dq) {
        // dv/dt
        dq[VOLTAGE] = q[CURRENT] / C;
        // di/dt
        dq[CURRENT] =
            qc * (Vdc / L) - ((R / L) * q[CURRENT]) - ((1.0 / L) * q[VOLTAGE]);
    }
    // Calculate the zero crossing functions to detect events
    void state_event_func(double const* q, double* z) {
        double Vz = V(q[VOLTAGE], q[CURRENT]);
        // Event at the interior tracking boundary
        z[0] = ci - Vz;
        // Event at the exterior tracking boundary
        z[1] = co - Vz;
        // Event at i = eps
        z[2] = q[CURRENT] - eps;
        // Event at i = -eps
        z[3] = q[CURRENT] + eps;
    }
    // Act at an event to change the discrete control mode
    void internal_event(double* q, bool const* state_event) {
        qc = control(q[VOLTAGE], q[CURRENT], qc);
    }
    // Don't need any of this stuff for this model
    double time_event_func(double const*) { return adevs_inf<double>(); }
    void postStep(double*) {}
    void external_event(double*, double, std::list<IO_Type> const &) {}
    void confluent_event(double*, bool const*, std::list<IO_Type> const &) {}
    void output_func(double const*, bool const*, std::list<IO_Type> &) {}

    ~inverter() {}
    // Get the current control mode
    int getControl() const { return qc; }
    // Get the desired frequency in Hz
    double getFreq() const { return w / (2.0 * pi); }
    // Get the tracking error
    double V(double v, double i) const {
        return ((i / a) * (i / a) + (v / b) * (v / b));
    }

  protected:
    double const R, L, C, Vdc, c, ci, co, w, a, b, eps;
    virtual int control(double v, double i, int q) = 0;

  private:
    int qc;
};

/**
 * Inverter extended with the control logic.
 */
class inverter_with_control : public inverter {
  public:
    inverter_with_control() : inverter() {}
    ~inverter_with_control() {}

  protected:
    // Control law
    int control(double v, double i, int q) {
        double Vz = V(v, i);
        // Rule (i)
        if (Vz >= co && ((i > eps && v < 0.0) || (i >= 0.0 && v >= 0.0)) &&
            q == 1) {
            return -1;
        }
        // Rule (ii)
        else if (Vz >= co &&
                 ((i < -eps && v > 0.0) || (i <= 0.0 && v <= 0.0)) && q == -1) {
            return 1;
        }
        // Rule (iii)
        else if (Vz <= ci && i >= 0.0) {
            return 1;
        }
        // Rule (iv)
        else if (Vz <= ci && i <= 0.0) {
            return -1;
        }
        // Rule (v)
        else if (Vz >= co && 0.0 <= i && i <= eps && v < 0.0 && q != 0) {
            return 0;
        }
        // Rule (vi)
        else if (Vz >= co && 0.0 >= i && i >= -eps && v > 0.0 && q != 0) {
            return 0;
        }
        // No change otherwise
        else {
            return q;
        }
    }
};

int main(int argc, char** argv) {
    // Get the initial conditions from the command line
    if (argc >= 4) {
        i0 = atof(argv[1]);
        v0 = atof(argv[2]);
        q0 = atoi(argv[3]);
    }
    // Set the number of inverter cycles to simulate
    int const sim_cycles = 50;
    // Create the model
    inverter_with_control* model = new inverter_with_control();
    // Calculate the ending time for the simulation
    double const end_time = double(sim_cycles) / model->getFreq();
    // Attach a numerical solver to the model
    Hybrid<IO_Type>* solver = new Hybrid<IO_Type>(
        model, new corrected_euler<IO_Type>(model, 1E-5, 1E-5),
        new linear_event_locator<IO_Type>(model, 1E-8));
    // Create the simulator
    Simulator<IO_Type>* sim = new Simulator<IO_Type>(solver);
    // Print the initial state
    std::cout << 0.0 << " ";
    std::cout << solver->getState(VOLTAGE) << " " << solver->getState(CURRENT) << " "
         << model->getControl() << " "
         << model->V(solver->getState(VOLTAGE), solver->getState(CURRENT))
         << std::endl;
    // Run the simulation, printing the time and state at each event
    while (sim->nextEventTime() < end_time) {
        std::cout << sim->nextEventTime() << " ";
        sim->execNextEvent();
        std::cout << solver->getState(VOLTAGE) << " " << solver->getState(CURRENT)
             << " " << model->getControl() << " "
             << model->V(solver->getState(VOLTAGE), solver->getState(CURRENT))
             << std::endl;
    }
    // Cleanup and exit
    delete sim;
    delete solver;
    return 0;
}
