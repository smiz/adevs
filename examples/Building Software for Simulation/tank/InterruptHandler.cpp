#include "InterruptHandler.h"
#include <iostream>
using namespace std;
using namespace adevs;

InterruptHandler::InterruptHandler(double freq)
    : Atomic<SimEvent>(),
      interrupt_period(1.0 / (8.0 * freq)),  // Initialize the model
      motor_voltage(7.2),
      exec_time(0.432E-6),
      ttg(interrupt_period),
      counter(0),
      left_on_time(0),
      right_on_time(0),
      reverse_left(false),
      reverse_right(false),
      last_left_v(0.0),
      last_right_v(0.0),
      left_v(0.0),
      right_v(0.0),
      phase(WAIT) {}

void InterruptHandler::delta_int() {
    // Start an interrupt
    if (phase == WAIT) {
        phase = EXEC;
        ttg = exec_time;
    }
    // End an interrupt and send the output
    else if (phase == EXEC) {
        // Increment the counter
        counter += 32;
        // Compute the next output voltage
        left_v = motor_voltage * (counter < left_on_time);
        if (reverse_left) {
            left_v = -left_v;
        }
        right_v = motor_voltage * (counter < right_on_time);
        if (reverse_right) {
            right_v = -right_v;
        }
        // Send the voltage and interrupt signal
        phase = OUTPUT;
        ttg = 0.0;
    }
    // Wait for the next interrupt
    else if (phase == OUTPUT) {
        // Remember the last output voltages
        last_left_v = left_v;
        last_right_v = right_v;
        // Wait for the next interrupt
        phase = WAIT;
        ttg = interrupt_period;
    }
}

void InterruptHandler::delta_ext(double e, list<SimEvent> const &xb) {
    // Decrement the time to go
    ttg -= e;
    // Look for input
    for (list<SimEvent>::iterator iter = xb.begin(); iter != xb.end(); iter++) {
        assert((*iter).getType() == SIM_MOTOR_ON_TIME);
        left_on_time = (*iter).simMotorOnTime().left;
        right_on_time = (*iter).simMotorOnTime().right;
        reverse_left = (*iter).simMotorOnTime().reverse_left;
        reverse_right = (*iter).simMotorOnTime().reverse_right;
    }
}

void InterruptHandler::delta_conf(list<SimEvent> const &xb) {
    delta_int();
    delta_ext(0.0, xb);
}

void InterruptHandler::output_func(list<SimEvent> &yb) {
    // If this is the end of an interrupt
    if (phase == OUTPUT) {
        // If the voltage changed, then send the new values
        if (last_left_v != left_v || last_right_v != right_v) {
            SimMotorVoltage volts;
            volts.el = left_v;
            volts.er = right_v;
            yb.push_back(SimEvent(volts));
        }
        // Send the interrupt indicator
        yb.push_back(SimEvent(SIM_INTERRUPT));
    }
    // If this is the start of an interrupt
    else if (phase == WAIT) {
        yb.push_back(SimEvent(SIM_INTERRUPT));
    }
}

double InterruptHandler::ta() {
    return ttg;
}
