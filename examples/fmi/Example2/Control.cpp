#include "Control.h"
#include "adevs_modelica_runtime.h"
using namespace adevs;

// This is used by the residual functions
static Control* active_model;


void Control::bound_params()
{
}

Control::Control(
    int extra_state_events, double eventHys):
    ode_system<OMC_ADEVS_IO_TYPE>(
        1+1, // Number of state variables plus one for the clock
        0+2*0+extra_state_events // Number of state event functions
    ),
    epsilon(eventHys),
    zc(NULL),
    samples(NULL),
    delays(NULL),
    eventFuncs(NULL)
 {
     timeValue = 0.0;
     if (numRelations() > 0)
         zc = new int[numRelations()];
     if (numTimeEvents() > 0)
     {
         samples = new AdevsSampleData*[numTimeEvents()];
         for (int i = 0; i < numTimeEvents(); i++)
             samples[i] = NULL;
     }
     if (numDelays() > 0)
     {
         delays = new AdevsDelayData*[numDelays()];
         for (int i = 0; i < numDelays(); i++)
             delays[i] = NULL;
     }
     if (numMathEvents() > 0)
     {
         eventFuncs = new AdevsMathEventFunc*[numMathEvents()];
         for (int i = 0; i < numMathEvents(); i++)
             eventFuncs[i] = NULL;
     }
 }

 Control::~Control()
 {
      if (zc != NULL) delete [] zc;
      if (samples != NULL)
      {
         for (int i = 0; i < numTimeEvents(); i++)
             if (samples[i] != NULL) delete samples[i];
         delete [] samples;
      }
      if (delays != NULL)
      {
         for (int i = 0; i < numDelays(); i++)
             if (delays[i] != NULL) delete delays[i];
         delete [] delays;
      }
      if (eventFuncs != NULL)
      {
         for (int i = 0; i < numMathEvents(); i++)
             if (eventFuncs[i] != NULL) delete eventFuncs[i];
         delete [] eventFuncs;
      }
 }
 
 static int residualFunc11(N_Vector y, N_Vector f, void*)
 {
   double* yd = NV_DATA_S(y);
   double* fd = NV_DATA_S(f);
   active_model->residualFunc11_cpp(yd,fd);
   return 0;
 }
 
 void Control::residualFunc11_cpp(double* y, double* res)
 {
   _qd2 = y[0];
   modelica_real tmp0;
   modelica_real tmp1;
   modelica_real tmp2;
   modelica_real tmp3;
   modelica_real tmp4;
   tmp0 = cos(_qd2);
   tmp1 = (_xd + (_xp + (_L * tmp0)));tmp2 = sin(_qd2);
   tmp3 = (_zd + (_L * tmp2));tmp4 = _l;res[0] = ((tmp1 * tmp1) + ((tmp3 * tmp3) - (tmp4 * tmp4)));
 }
 
 void Control::solve_residualFunc11_cpp()
 {
     int flag;
     int NEQ = 1;
     N_Vector y = N_VNew_Serial(NEQ);
     N_Vector scale = N_VNew_Serial(NEQ);
     void* kmem = KINCreate();
     active_model = this;
     assert(kmem != NULL);
     flag = KINInit(kmem, residualFunc11, y);
     assert(flag == KIN_SUCCESS);
     flag = KINDense(kmem, NEQ);
     assert(flag == KIN_SUCCESS);
     N_VConst_Serial(1.0,scale);
     NV_Ith_S(y,0) = _qd2;
     flag = KINSol(kmem,y,KIN_LINESEARCH,scale,scale);
     // Save the outcome and calculate any dependent variables
     residualFunc11(y,scale,NULL);
     N_VDestroy_Serial(y);
     N_VDestroy_Serial(scale);
     KINFree(&kmem);
 }
 
 static int residualFunc13(N_Vector y, N_Vector f, void*)
 {
   double* yd = NV_DATA_S(y);
   double* fd = NV_DATA_S(f);
   active_model->residualFunc13_cpp(yd,fd);
   return 0;
 }
 
 void Control::residualFunc13_cpp(double* y, double* res)
 {
   _qd1 = y[0];
   modelica_real tmp5;
   modelica_real tmp6;
   modelica_real tmp7;
   modelica_real tmp8;
   modelica_real tmp9;
   tmp5 = cos(_qd1);
   tmp6 = (_xd + ((-_xp) - (_L * tmp5)));tmp7 = sin(_qd1);
   tmp8 = (_zd + (_L * tmp7));tmp9 = _l;res[0] = ((tmp6 * tmp6) + ((tmp8 * tmp8) - (tmp9 * tmp9)));
 }
 
 void Control::solve_residualFunc13_cpp()
 {
     int flag;
     int NEQ = 1;
     N_Vector y = N_VNew_Serial(NEQ);
     N_Vector scale = N_VNew_Serial(NEQ);
     void* kmem = KINCreate();
     active_model = this;
     assert(kmem != NULL);
     flag = KINInit(kmem, residualFunc13, y);
     assert(flag == KIN_SUCCESS);
     flag = KINDense(kmem, NEQ);
     assert(flag == KIN_SUCCESS);
     N_VConst_Serial(1.0,scale);
     NV_Ith_S(y,0) = _qd1;
     flag = KINSol(kmem,y,KIN_LINESEARCH,scale,scale);
     // Save the outcome and calculate any dependent variables
     residualFunc13(y,scale,NULL);
     N_VDestroy_Serial(y);
     N_VDestroy_Serial(scale);
     KINFree(&kmem);
 }

 static void static_initial_objective_func(long*, double* w, double* f)
 {
     active_model->initial_objective_func(w,f,1.0);
 }
 
 void Control::initial_objective_func(double* w, double *f, double $P$_lambda)
 {
     // Get new values for the unknown variables
     for (unsigned i = 0; i < init_unknown_vars.size(); i++)
     {
         if (w[i] != w[i]) MODELICA_TERMINATE("could not initialize unknown reals");
         *(init_unknown_vars[i]) = w[i];
     }
     // Calculate new state variable derivatives and algebraic variables
     bound_params();
     selectStateVars();
     calc_vars(NULL,true);
     // Calculate the new value of the objective function
     double r = 0.0;
     *f = 0.0;
     r=_$dummy-_PRE_$dummy; *f+=r*r;
     r=_DER_$dummy-_PRE_DER_$dummy; *f+=r*r;
     r=_pi-_PRE_pi; *f+=r*r;
     r=_xp-_PRE_xp; *f+=r*r;
     r=_L-_PRE_L; *f+=r*r;
     r=_l-_PRE_l; *f+=r*r;
 }
 
 void Control::solve_for_initial_unknowns()
 {
   init_unknown_vars.push_back(&_qd2);
   init_unknown_vars.push_back(&_qd1);
   init_unknown_vars.push_back(&_zd);
   init_unknown_vars.push_back(&_xd);
   if (!init_unknown_vars.empty())
   {
       long N = init_unknown_vars.size();
       long NPT = 2*N+2;
       double* w = new double[N];
       for (unsigned i = 0; i < init_unknown_vars.size(); i++)
           w[i] = *(init_unknown_vars[i]);
       double RHOBEG = 10.0;
       double RHOEND = 1.0E-7;
       long IPRINT = 0;
       long MAXFUN = 50000;
       double* scratch = new double[(NPT+13)*(NPT+N)+3*N*(N+3)/2];
       active_model = this;
       newuoa_(&N,&NPT,w,&RHOBEG,&RHOEND,&IPRINT,&MAXFUN,scratch,
               static_initial_objective_func);
       delete [] w;
       delete [] scratch;
   }
 }

 void Control::clear_event_flags()
 {
     for (int i = 0; i < numRelations(); i++) zc[i] = -1;
     for (int i = 0; i < numMathEvents(); i++)
         if (eventFuncs[i] != NULL) eventFuncs[i]->setInit(true);
 }
 
 void Control::init(double* q)
 {
     atInit = true;
     atEvent = false;
     timeValue = q[numVars()-1] = 0.0;
     clear_event_flags();
     // Get initial values as given in the model
            _pi=3.141592653589793;//_pi
            _xp=0.1;//_xp
            _L=0.3;//_L
            _l=0.7;//_l
             _$dummy=0.0;
             _qd2=0.0;//_qd2
             _qd1=0.0;//_qd1
             _zd=0.0;//_zd
             _xd=0.0;//_xd
             _DER_$dummy=0.0;
     // Save these to the old values so that pre() and edge() work
     save_vars();
     // Calculate any equations that provide initial values
     bound_params();
     // Solve for any remaining unknowns
     solve_for_initial_unknowns();
     selectStateVars();
     calc_vars();
     save_vars();
     q[0]=_$dummy;
     atInit = false;
     for (int i = 0; i < numMathEvents(); i++)
         if (eventFuncs[i] != NULL) eventFuncs[i]->setInit(false);
 }

 void Control::der_func(const double* q, double* dq)
 {
     calc_vars(q);
     dq[0]=_DER_$dummy;
     dq[numVars()-1] = 1.0;
     restore_vars();
 }

 void Control::postStep(double* q)
 {
     calc_vars(q);
     if (selectStateVars())
     {
         q[0] = _$dummy;
         calc_vars(q,true);
     }
     save_vars();
 }

 void Control::save_vars()
 {
   _PRE_timeValue = timeValue;
   _PRE_$dummy=_$dummy;
   _PRE_DER_$dummy=_DER_$dummy;
   _PRE_qd2=_qd2;
   _PRE_qd1=_qd1;
   _PRE_zd=_zd;
   _PRE_xd=_xd;
   _PRE_pi=_pi;
   _PRE_xp=_xp;
   _PRE_L=_L;
   _PRE_l=_l;
 }

 void Control::restore_vars()
 {
   timeValue = _PRE_timeValue;
   _$dummy=_PRE_$dummy;
   _DER_$dummy=_PRE_DER_$dummy;
   _qd2=_PRE_qd2;
   _qd1=_PRE_qd1;
   _zd=_PRE_zd;
   _xd=_PRE_xd;
     _pi=_PRE_pi;
     _xp=_PRE_xp;
     _L=_PRE_L;
     _l=_PRE_l;
 }

 void Control::calc_vars(const double* q, bool doReinit)
 {
     bool reInit = false;
     active_model = this;
     if (atEvent || doReinit) clear_event_flags();
     // Copy state variable arrays to values used in the odes
     if (q != NULL)
     {
         timeValue = q[numVars()-1];
         _$dummy=q[0];
     }
     modelica_real tmp10;
     modelica_real tmp11;
     // Primary equations
     _DER_$dummy = 0.0; 
     tmp10 = cos((0.5 * (_pi * timeValue)));
     _zd = (-0.7 + (0.1 * tmp10)); 
     tmp11 = sin((0.5 * (_pi * timeValue)));
     _xd = (-0.35 * tmp11); 
     solve_residualFunc11_cpp();
     solve_residualFunc13_cpp();
     // Alias equations
     // Reinits
     // Alias assignments
     if (atEvent && !reInit) reInit = check_for_new_events();
     if (reInit)
     {
         save_vars();
         calc_vars(NULL,reInit);
     }
 }

 
 bool Control::check_for_new_events()
 {
   bool result = false;
   double* z = new double[numZeroCrossings()];
   for (int i = 0; i < numRelations() && !result; i++)
   {
     if (z[i] < 0.0 && zc[i] == 1) result = true;
     else if (z[i] > 0.0 && zc[i] == 0) result = true;
   }
   for (int i = numRelations(); i < numZeroCrossings() && !result; i += 2)
   {
       if (z[i] < 0.0 || z[i+1] < 0.0) result = true;
   }
   delete [] z;
   return result;
 }
 
 void Control::state_event_func(const double* q, double* z)
 {
     calc_vars(q);
     extra_state_event_funcs(&(z[numStateEvents()]));
     restore_vars();
 }
 
 bool Control::sample(int index, double tStart, double tInterval)
 {
   index--;
   assert(index >= 0);
     if (samples[index] == NULL)
         samples[index] = new AdevsSampleData(tStart,tInterval);
     return samples[index]->atEvent(timeValue,epsilon);
 }
 
 double Control::time_event_func(const double* q)
 {
     double ttgMin = adevs_inf<double>();
     for (int i = 0; i < numTimeEvents(); i++)
     {
         double ttg = samples[i]->timeToEvent(timeValue);
         if (ttg < ttgMin) ttgMin = ttg;
     }
     for (int i = 0; i < numDelays(); i++)
     {
         double ttg = delays[i]->getMaxDelay();
         if (ttg < ttgMin) ttgMin = ttg;
     }
     return ttgMin;
 }
 
 void Control::internal_event(double* q, const bool* state_event)
 {
     atEvent = true;
     for (int i = 0; i < numTimeEvents(); i++)
     {
         assert(samples[i] != NULL);
         samples[i]->setEnabled(true);
     }
     calc_vars(q);
     for (int i = 0; i < numTimeEvents(); i++)
     {
         samples[i]->update(timeValue,epsilon);
         samples[i]->setEnabled(false);
     }
     save_vars(); // save the new state of the model
     // Reinitialize state variables that need to be reinitialized
     q[0]=_$dummy;
     for (int i = 0; i < numMathEvents(); i++)
         if (eventFuncs[i] != NULL) eventFuncs[i]->setInit(false);
     atEvent = false;
 }
 
 double Control::floor(double expr, int index)
 {
     if (eventFuncs[index] == NULL)
         eventFuncs[index] = new AdevsFloorFunc(epsilon);
     return eventFuncs[index]->calcValue(expr);
 }
 
 double Control::div(double x, double y, int index)
 {
     if (eventFuncs[index] == NULL)
         eventFuncs[index] = new AdevsDivFunc(epsilon);
     return eventFuncs[index]->calcValue(x/y);
 }
 
 int Control::integer(double expr, int index)
 {
     if (eventFuncs[index] == NULL)
         eventFuncs[index] = new AdevsFloorFunc(epsilon);
     return int(eventFuncs[index]->calcValue(expr));
 }
 
 double Control::ceil(double expr, int index)
 {
     if (eventFuncs[index] == NULL)
         eventFuncs[index] = new AdevsCeilFunc(epsilon);
     return eventFuncs[index]->calcValue(expr);
 }


 bool Control::selectStateVars()
 {
     bool doReinit = false;
     return doReinit;
 }
 
 double Control::calcDelay(int index, double expr, double t, double delay)
 {
     if (delays[index] == NULL || !delays[index]->isEnabled()) return expr;
     else return delays[index]->sample(t-delay);
 }
 
 void Control::saveDelay(int index, double expr, double t, double max_delay)
  {
      if (delays[index] == NULL)
          delays[index] = new AdevsDelayData(max_delay);
      delays[index]->insert(t,expr);
  }
 
