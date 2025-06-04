#include <iostream>
#include <typeinfo>
#include "adevs/adevs.h"
#include "adevs/solvers/cvode.h"
#include <nvector/nvector_serial.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <sunmatrix/sunmatrix_dense.h>
#include "check_ball1d_solution.h"
#include "sampler.h"
using namespace std;
using namespace adevs;

#define FALL 0
#define CLIMB 1

/**
 * Simple test which simulates a bouncing ball. The output value is the height of
 * the ball. Input values cause the system to produce an output sample immediately.
 */
class bouncing_ball : public CVODE<double> {
  public:
    bouncing_ball(double h_max):
    CVODE<double>(),
    phase(FALL),
    sample(false),
    t(0.0),
    h_max(h_max) {
        int retval;
        y = N_VNew_Serial(2);
        abstol = N_VNew_Serial(2);
        NV_Ith_S(y,0) = 1.0;  // Initial height
        NV_Ith_S(y,1) = 0.0;  // Initial velocity
        NV_Ith_S(abstol,0) = 1E-8;  // Height error tolerance
        NV_Ith_S(abstol,1) = 1E-8;  // Velocity error tolerance
        cvode_mem = CVodeCreate(CV_ADAMS);
        retval = CVodeInit(cvode_mem,bouncing_ball::f,0.0,y);
        assert(retval == CV_SUCCESS);
        retval = CVodeSVtolerances(cvode_mem,1E-8,abstol);
        assert(retval == CV_SUCCESS);
        retval = CVodeRootInit(cvode_mem,1,bouncing_ball::g);
        assert(retval == CV_SUCCESS);
        A = SUNDenseMatrix(2,2);
        LS = SUNLinSol_Dense(y,A);
        retval = CVodeSetLinearSolver(cvode_mem,LS,A);
        assert(retval == CV_SUCCESS);
        retval = CVodeSetUserData(cvode_mem,(void*)(&phase));
        assert(retval == CV_SUCCESS);
    }
    ~bouncing_ball() {
        N_VDestroy(y);
        N_VDestroy(abstol);
        CVodeFree(&cvode_mem);
        SUNLinSolFree(LS);
        SUNMatDestroy(A);
    }

    void cvode_delta_int() {
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
    void cvode_delta_ext(double t, std::list<PinValue<double>> const &) {
        this->t = t;
        sample = true;
    }

    void cvode_delta_conf(std::list<PinValue<double>> const &) {
        sample = true;
    }

    void cvode_output_func(std::list<PinValue<double>> &yb) {
        PinValue<double> event(sample_pin, NV_Ith_S(y,0));
        yb.push_back(event);
    }
    N_Vector cvode_get_state() { return y; };

    void cvode_integrate(double& tf, bool& event) {
        if (sample) {
            tf = t;
            event = true;
            return;
        }
        int retval = CVode(cvode_mem,t+h_max,y,&t,CV_NORMAL);
        assert(retval == CV_SUCCESS || retval == CV_ROOT_RETURN);
        retval = CVodeGetRootInfo(cvode_mem,&event_flag);
        assert(retval == CV_SUCCESS);
        tf = t;
        event = (event_flag != 0);
    }

    void cvode_integrate_until(double tf, bool& event) {
        int retval = CVode(cvode_mem,tf,y,&t,CV_NORMAL);
        assert(retval == CV_SUCCESS);
        retval = CVodeGetRootInfo(cvode_mem,&event_flag);
        assert(retval == CV_SUCCESS);
        assert(t == tf);
        event = (event_flag != 0);
    }

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
    N_Vector y;
    N_Vector abstol;
    SUNMatrix A;
    SUNLinearSolver LS;
    void* cvode_mem;
};

int bouncing_ball::f(double, N_Vector y, N_Vector ydot, void*) {
    NV_Ith_S(ydot,0) = NV_Ith_S(y,1);
    NV_Ith_S(ydot,1) = -2.0;  // For test case
    return 0;
}

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
    auto graph = make_shared<adevs::Graph<double>>();
    auto ball = make_shared<bouncing_ball>(h_max);
    auto sample = make_shared<sampler>(0.01);
    graph->add_atomic(ball);
    graph->add_atomic(sample);
    graph->connect(sample->sample_pin,ball);
    auto checker = make_shared<SolutionChecker>(ball.get(),to_cout);
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