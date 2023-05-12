#include "circuit.h"
#include <cmath>
using namespace adevs;

#define IFA 0 // Current through A inductor of the filter
#define ILA 1 // Current through A inductor of the line
#define VMA 2 // Voltage at midpoint of A
#define IFB 3 // Current through B inductor of the filter
#define ILB 4 // Current through B inductor of the line
#define VMB 5 // Voltage at midpoint of B
#define IFC 6 // Current through C inductor of the filter
#define ILC 7 // Current through C inductor of the line
#define VMC	8 // Voltage at midpoint of C

/// Angles of each power phase
const double Circuit::phase_angle[3] =
{
	0.0,
	120.0*M_PI/180.0,
	240.0*M_PI/180.0
};

extern "C"
{
	/// Linear system solver
	void dgelsy_(int*,int*,int*,double*,int*,double*,int*,int*,double*,int*,double*,int*,int*);
   /// Matrix multiplication
	void dgemv_(char*,int*,int*,double*,double*,int*,double*,int*,double*,double*,int*);
};

Circuit::Circuit(
	double C,
	double Linv,
	double Ls,
	double fs,
	double Is,
	double Rs,
	double Vinv,
	double sample_rate):
	Atomic<event_t>(),
	C(C),
	Linv(Linv),
	Vinv(Vinv),
	Ls(Ls),
	Fs(fs),
	Is(Is),
	Rs(Rs),
	sample_period(1.0/sample_rate),
	time(0.0),
	ts(sample_period)
{
	TRANS = 'N';
	N = M = NRHS = 3;
	LDA = LDB = RANK = M;
	INCX = 1;
	RCOND = 1E-6;
	LWORK = -1;
	WORK = new double[1];	
	dgelsy_(&M,&N,&NRHS,Aleft,&LDA,qnext,&LDB,JPVT,&RCOND,&RANK,WORK,&LWORK,&info);
	LWORK = (int)(WORK[0]);
	delete [] WORK;
	WORK = new double[LWORK];
	// State transition matrix
	A[0] = 0.0;
	A[1] = 0.0;
	A[2] = 1.0/C;
	A[3] = 0.0;
	A[4] = -Rs/Ls;
	A[5] = 0.0;
	A[6] = -1.0/Linv;
	A[7] = 1.0/Ls;
	A[8] = -1.0/C;	
	// Switches are open (ground)
	for (int i = 0; i < 3; i++)
		polarity[i] = 0.0;
	// Currents and voltages are zero
	for (int i = 0; i < 9; i++)
		q[1][i] = q[0][i] = 0.0;
	qnow = q[0];
	qnext = q[1];
}

Circuit::~Circuit()
{
	delete [] WORK;
}

/// Main current from the grid at 60 Hz
double Circuit::current_main(double t, int i) const
{
	return Is*cos(2.0*M_PI*Fs*t+phase_angle[i]);
}

/// Main current plus the harmonic to compensate
double Circuit::current_grid(double t, int i)
{
	return current_main(t,i)+current_comp(t,i,phase_angle[i]);
}

void Circuit::integrate(double h)
{
	double alpha = h/2.0, beta = 1.0;
	/// Solve system using a trapezoidal step
	/// (1-hA/2)q_{n+1} = (1+hA/2)*q_n+(h/2)(f_n+f_{n+1})
	for (int i = 0; i < 3; i++)
	{
		// Calculte (h/2)(f_n+f_{n+1})+q_n
		double Vswitch = Vinv*polarity[i];
		qnext[3*i] = qnow[3*i]+h*Vswitch/Linv;
		qnext[3*i+1] = qnow[3*i+1]-(h/2.0)*Rs*(current_grid(time,i)+current_grid(time+h,i))/Ls;
		qnext[3*i+2] = qnow[3*i+2];
		// Finish the calculation of the right hand side. Solution is in f.
		dgemv_(&TRANS,&M,&N,&alpha,A,&LDA,qnow+3*i,&INCX,&beta,qnext+3*i,&INCX);
	}
	// find q_{n+1}
	for (int k = 0; k < 9; k++)
		Aleft[k] = -A[k]*h/2.0;
	Aleft[0] += 1.0;
	Aleft[4] += 1.0;
	Aleft[8] += 1.0;
	JPVT[0] = JPVT[1] = JPVT[2] = 0;
	dgelsy_(&M,&N,&NRHS,Aleft,&LDA,qnext,&LDB,JPVT,&RCOND,&RANK,WORK,&LWORK,&info);
	assert(info == 0);
}

double Circuit::ta()
{
	return ts;
}

void Circuit::delta_int()
{
	time += ts;
	ts = sample_period;
	::swap(qnow,qnext);
}

void Circuit::delta_conf(const adevs::Bag<event_t>& xb)
{
	time += ts;
	ts = sample_period;
	::swap(qnow,qnext);
	set_switches(xb);
}

void Circuit::delta_ext(double e, const adevs::Bag<event_t>& xb)
{
	integrate(e);
	time += e;
	ts -= e;
	::swap(qnow,qnext);
	set_switches(xb);
}

void Circuit::set_switches(const adevs::Bag<event_t>& xb)
{
	// Only input are switch states. We take the first one.
	for (auto x: xb)
	{
		int i = x.value.open_close.channel;
		polarity[i] = x.value.open_close.polarity;
	}
}

/// Output function produces samples
void Circuit::output_func(adevs::Bag<event_t>& yb)
{
	event_t y;
	integrate(ts);
	y.type = ABC_SAMPLE;
	double ia = current_grid(time+ts,0);
	double ib = current_grid(time+ts,1);
	double ic = current_grid(time+ts,2);
	// Voltage across the load resistor
	double va = (qnext[ILA]+ia)*Rs;
	double vb = (qnext[ILB]+ib)*Rs;
	double vc = (qnext[ILC]+ic)*Rs;
	y.value.data.vabc[0] = va;
	y.value.data.vabc[1] = vb;
	y.value.data.vabc[2] = vc;
	// Current through the load resistor
	y.value.data.iabc[0] = qnext[ILA]+ia;
	y.value.data.iabc[1] = qnext[ILB]+ib;
	y.value.data.iabc[2] = qnext[ILC]+ic;
	yb.insert(y);
}

