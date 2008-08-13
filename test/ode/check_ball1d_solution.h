#ifndef check_ball1d_solution_h_
#define check_ball1d_solution_h_

/**
 * Returns abs error in the computed solution of h(t).
 * Solution is to dh/dt = v, dv/dt = -2, h(0) = 1, v(0) = 1
 */
double check_ball1d_solution(double t, double h);
// True if solution within acceptable limit
bool ball1d_soln_ok(double t, double h);

#endif
