#include <iostream>
#include <typeinfo>
#include "adevs/adevs.h"
#include "adevs/solvers/cvode.h"
#include <nvector/nvector_serial.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <sunmatrix/sunmatrix_dense.h>
#include "check_ball1d_solution.h"
#include "sampler.h"

using namespace adevs;

#define FALL 0
#define CLIMB 1

/**
 * Simple test which simulates a bouncing ball. The output value is the height
 * of the ball. Input cause the system to produce an output sample immediately.
 */
class bouncing_ball : public CVODE<double> {
  public:
    /** 
     * Constructor initializes the CVODE system.
     * 
     * @param h_max The maximum step size that CVODE should take
     */
    bouncing_ball(double h_max):
    CVODE<double>(),
    phase(FALL),
    sample(false),
    t(0.0),
    h_max(h_max) {
        int retval;

#if SUNDIALS_VERSION_MAJOR < 7
        SUNContext_Create(nullptr,&sunctx);
#else // SUNDIALS_VERSION_MAJOR >= 7
        SUNContext_Create(0,&sunctx);
#endif

        y = N_VNew_Serial(2,sunctx);
        abstol = N_VNew_Serial(2,sunctx);
        NV_Ith_S(y,0) = 1.0;  // Initial height
        NV_Ith_S(y,1) = 0.0;  // Initial velocity
        NV_Ith_S(abstol,0) = 1E-8;  // Height error tolerance
        NV_Ith_S(abstol,1) = 1E-8;  // Velocity error tolerance
        cvode_mem = CVodeCreate(CV_ADAMS,sunctx);
        retval = CVodeInit(cvode_mem,bouncing_ball::f,0.0,y);
        assert(retval == CV_SUCCESS);
        retval = CVodeSVtolerances(cvode_mem,1E-8,abstol);
        assert(retval == CV_SUCCESS);
        retval = CVodeRootInit(cvode_mem,1,bouncing_ball::g);
        assert(retval == CV_SUCCESS);
        A = SUNDenseMatrix(2,2,sunctx);
        LS = SUNLinSol_Dense(y,A,sunctx);
        retval = CVodeSetLinearSolver(cvode_mem,LS,A);
        assert(retval == CV_SUCCESS);
        retval = CVodeSetUserData(cvode_mem,(void*)(&phase));
        assert(retval == CV_SUCCESS);
    }
    /**
     * Cleanup the CVODE system
     */
    ~bouncing_ball() {
        N_VDestroy(y);
        N_VDestroy(abstol);
        CVodeFree(&cvode_mem);
        SUNLinSolFree(LS);
        SUNMatDestroy(A);
        SUNContext_Free(&sunctx);
    }

    /**
     * The internal transition function. This is called when
     * the ball bounces and when it generates a sample following
     * the receipt of an input.
     */
    void cvode_delta_int() {
        /// Did we bounce?
        if (event_flag) {
            if (phase == FALL)  // Hit the ground
            {
                phase = CLIMB;
                NV_Ith_S(y,1) = -NV_Ith_S(y,1);
                int retval = CVodeReInit(cvode_mem,t,y);
                assert(retval == CV_SUCCESS);
            } else  { // reach apogee
                phase = FALL;
            }
        }
        sample = false;
    }
    /**
     * Schedule an output immediately in response to an input.
     */
    void cvode_delta_ext(double t, std::list<PinValue<double>> const &) {
        this->t = t;
        sample = true; // tell cvode_integrate() to take a step size of zero
    }

    /**
     * Schedule an output immediately in response to an input.
     */
    void cvode_delta_conf(std::list<PinValue<double>> const &) {
        sample = true; // tell cvode_integrate() to take a step size of zero
    }

    /**
     * Send the height of the ball and an output
     */
    void cvode_output_func(std::list<PinValue<double>> &yb) {
        PinValue<double> event(sample_pin, NV_Ith_S(y,0));
        yb.push_back(event);
    }

    N_Vector cvode_get_state() { return y; };

    /**
     * Run an integration step
     */
    void cvode_integrate(double& tf, bool& event) {
        // If we are sampling, the step size is zero and
        // we want an internal event
        if (sample) {
            tf = t; // Next time is current time
            event = true; // We want an internal event
            return;
        }
        // Integrate up to our maximum step size and check for state events
        int retval = CVode(cvode_mem,t+h_max,y,&t,CV_NORMAL);
        assert(retval == CV_SUCCESS || retval == CV_ROOT_RETURN);
        retval = CVodeGetRootInfo(cvode_mem,&event_flag);
        assert(retval == CV_SUCCESS);
        // Return the time that we integrated up to
        tf = t;
        // Are there any state events at that time?
        event = (event_flag != 0);
    }

    /// Integrate until exactly time tf and report if there are
    /// any state events at that time
    void cvode_integrate_until(double tf, bool& event) {
        int retval = CVode(cvode_mem,tf,y,&t,CV_NORMAL);
        assert(retval == CV_SUCCESS);
        retval = CVodeGetRootInfo(cvode_mem,&event_flag);
        assert(retval == CV_SUCCESS);
        assert(t == tf);
        event = (event_flag != 0);
    }

    /// Reinitialize continuous variables following and event
    void cvode_reinit(N_Vector y, double t) {
        int retval = CVodeReInit(cvode_mem,t,y);
        assert(retval == CV_SUCCESS);
        this->t = t;
        N_VAddConst(y,0.0,this->y);
    }

    double getHeight() const { return NV_Ith_S(y,0); }
    static int f(double t, N_Vector y, N_Vector ydot, void*);
    static int g(double t, N_Vector y, double* gout, void* phase);

    const pin_t sample_pin;

    private:
    int phase, event_flag;
    bool sample;
    double t;
    const double h_max;
    SUNContext sunctx;
    N_Vector y;
    N_Vector abstol;
    SUNMatrix A;
    SUNLinearSolver LS;
    void* cvode_mem;
};

/// dy/dt = f(y)
int bouncing_ball::f(double, N_Vector y, N_Vector ydot, void*) {
    NV_Ith_S(ydot,0) = NV_Ith_S(y,1);
    NV_Ith_S(ydot,1) = -2.0;  // For test case
    return 0;
}

/// this is the state event detection function
int bouncing_ball::g(double, N_Vector y, double* gout, void* user_data) {
    int phase = *((int*)user_data);
    if (phase == FALL) {
        gout[0] = NV_Ith_S(y,0);  // Bounce if it is going down
    } else {
        gout[0] = NV_Ith_S(y,1);  // Start falling at apogee
    }
    return 0;
}

class SolutionChecker : public EventListener<double> {
  public:
    SolutionChecker(bouncing_ball* ball, bool to_cout) : EventListener<double>(),ball(ball),to_cout(to_cout) {}
    void outputEvent(Atomic<double>&, PinValue<double>&, double){}
    void inputEvent(Atomic<double>&, PinValue<double>&, double){}
    void stateChange(Atomic<double>& model, double t) {
        if (ball == &model) {
            assert(ball1d_soln_ok(t, ball->getHeight()));
        }
        if (to_cout) {
            std::cout << t << " " << ball->getHeight() << std::endl;
        }
    }
  private:
    bouncing_ball* ball;
    const bool to_cout;
};

void run_solution(double h_max, bool to_cout) {
    auto graph = std::make_shared<adevs::Graph<double>>();
    auto ball = std::make_shared<bouncing_ball>(h_max);
    auto sample = std::make_shared<sampler>(0.01);
    graph->add_atomic(ball);
    graph->add_atomic(sample);
    graph->connect(sample->sample_pin,ball);
    auto checker = std::make_shared<SolutionChecker>(ball.get(),to_cout);
    adevs::Simulator<double> sim(graph);
    sim.addEventListener(checker);
    while (sim.nextEventTime() < 10.0) {
        sim.execNextEvent();
    }
}

int main() {
    // Test with a whole mess of step sizes
    for (int i = 0; i <= 100; i++) {
        double h = 0.001*rand()/RAND_MAX*0.999;
        run_solution(h,i == 1000);
    }
    return 0;
}
