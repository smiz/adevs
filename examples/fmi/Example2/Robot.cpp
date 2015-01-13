#include "Robot.h"
#include "adevs_modelica_runtime.h"
using namespace adevs;

// This is used by the residual functions
static Robot* active_model;


void Robot::bound_params()
{
    _JqInv.get(0,1) = 0.0; 
    _JqInv.get(1,0) = 0.0; 
}

Robot::Robot(
    int extra_state_events, double eventHys):
    ode_system<OMC_ADEVS_IO_TYPE>(
        4+1, // Number of state variables plus one for the clock
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

 Robot::~Robot()
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
 
 static int residualFunc38(N_Vector y, N_Vector f, void*)
 {
   double* yd = NV_DATA_S(y);
   double* fd = NV_DATA_S(f);
   active_model->residualFunc38_cpp(yd,fd);
   return 0;
 }
 
 void Robot::residualFunc38_cpp(double* y, double* res)
 {
   _z = y[0];
   _q1 = y[1];
   _x = y[2];
   _q2 = y[3];
   modelica_real tmp0;
   modelica_real tmp1;
   modelica_real tmp2;
   modelica_real tmp3;
   modelica_real tmp4;
   modelica_real tmp5;
   modelica_real tmp6;
   modelica_real tmp7;
   modelica_real tmp8;
   modelica_real tmp9;
   _$TMP_33.assign((modelica_real)(_$STATESET2_x.get(0) + (((((-((modelica_real)(modelica_integer)_$STATESET2_A.get(0,0))) * _z) + ((-((modelica_real)(modelica_integer)_$STATESET2_A.get(0,1))) * _q1)) + ((-((modelica_real)(modelica_integer)_$STATESET2_A.get(0,2))) * _x)) + ((-((modelica_real)(modelica_integer)_$STATESET2_A.get(0,3))) * _q2))),(modelica_real)(_$STATESET2_x.get(1) + (((((-((modelica_real)(modelica_integer)_$STATESET2_A.get(1,0))) * _z) + ((-((modelica_real)(modelica_integer)_$STATESET2_A.get(1,1))) * _q1)) + ((-((modelica_real)(modelica_integer)_$STATESET2_A.get(1,2))) * _x)) + ((-((modelica_real)(modelica_integer)_$STATESET2_A.get(1,3))) * _q2))));
   tmp0 = cos(_q2);
   tmp1 = (_x + (_xp + (_L * tmp0)));tmp2 = sin(_q2);
   tmp3 = (_z + (_L * tmp2));tmp4 = _l;res[0] = ((tmp1 * tmp1) + ((tmp3 * tmp3) - (tmp4 * tmp4)));
   tmp5 = cos(_q1);
   tmp6 = (_x + ((-_xp) - (_L * tmp5)));tmp7 = sin(_q1);
   tmp8 = (_z + (_L * tmp7));tmp9 = _l;res[1] = ((tmp6 * tmp6) + ((tmp8 * tmp8) - (tmp9 * tmp9)));
   res[2] = _$TMP_33.get(0);
   res[3] = _$TMP_33.get(1);
 }
 
 void Robot::solve_residualFunc38_cpp()
 {
     int flag;
     int NEQ = 4;
     N_Vector y = N_VNew_Serial(NEQ);
     N_Vector scale = N_VNew_Serial(NEQ);
     void* kmem = KINCreate();
     active_model = this;
     assert(kmem != NULL);
     flag = KINInit(kmem, residualFunc38, y);
     assert(flag == KIN_SUCCESS);
     flag = KINDense(kmem, NEQ);
     assert(flag == KIN_SUCCESS);
     N_VConst_Serial(1.0,scale);
     NV_Ith_S(y,0) = _z;
     NV_Ith_S(y,1) = _q1;
     NV_Ith_S(y,2) = _x;
     NV_Ith_S(y,3) = _q2;
     flag = KINSol(kmem,y,KIN_LINESEARCH,scale,scale);
     // Save the outcome and calculate any dependent variables
     residualFunc38(y,scale,NULL);
     N_VDestroy_Serial(y);
     N_VDestroy_Serial(scale);
     KINFree(&kmem);
 }

 static void static_initial_objective_func(long*, double* w, double* f)
 {
     active_model->initial_objective_func(w,f,1.0);
 }
 
 void Robot::initial_objective_func(double* w, double *f, double $P$_lambda)
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
      r = _x; *f += r*r;
      r = (0.6 + _z); *f += r*r;
     r=_T.get(0)-_PRE_T.get(0); *f+=r*r;
     r=_T.get(1)-_PRE_T.get(1); *f+=r*r;
     r=_pi-_PRE_pi; *f+=r*r;
     r=_xp-_PRE_xp; *f+=r*r;
     r=_L-_PRE_L; *f+=r*r;
     r=_l-_PRE_l; *f+=r*r;
     r=_m1-_PRE_m1; *f+=r*r;
     r=_m2-_PRE_m2; *f+=r*r;
     r=_m3-_PRE_m3; *f+=r*r;
     r=_ml-_PRE_ml; *f+=r*r;
     r=_Jmot-_PRE_Jmot; *f+=r*r;
     r=_Jred-_PRE_Jred; *f+=r*r;
     r=_I-_PRE_I; *f+=r*r;
     r=_Fs-_PRE_Fs; *f+=r*r;
     r=_Fv-_PRE_Fv; *f+=r*r;
     r=_nu-_PRE_nu; *f+=r*r;
     r=_g-_PRE_g; *f+=r*r;
     r=_sampleFreq-_PRE_sampleFreq; *f+=r*r;
 }
 
 void Robot::solve_for_initial_unknowns()
 {
   init_unknown_vars.push_back(&_$STATESET1_x.get(0));
   init_unknown_vars.push_back(&_$STATESET1_x.get(1));
   init_unknown_vars.push_back(&_$STATESET2_x.get(0));
   init_unknown_vars.push_back(&_$STATESET2_x.get(1));
   init_unknown_vars.push_back(&_DER_$STATESET1_x.get(0));
   init_unknown_vars.push_back(&_DER_$STATESET1_x.get(1));
   init_unknown_vars.push_back(&_DER_$STATESET2_x.get(0));
   init_unknown_vars.push_back(&_DER_$STATESET2_x.get(1));
   init_unknown_vars.push_back(&_M.get(0));
   init_unknown_vars.push_back(&_M.get(1));
   init_unknown_vars.push_back(&_JxT.get(0,0));
   init_unknown_vars.push_back(&_JxT.get(0,1));
   init_unknown_vars.push_back(&_JxT.get(1,0));
   init_unknown_vars.push_back(&_JxT.get(1,1));
   init_unknown_vars.push_back(&_Ttplate.get(0));
   init_unknown_vars.push_back(&_Ttplate.get(1));
   init_unknown_vars.push_back(&_Tfarm.get(0));
   init_unknown_vars.push_back(&_Tfarm.get(1));
   init_unknown_vars.push_back(&_Tarm.get(0));
   init_unknown_vars.push_back(&_Tarm.get(1));
   init_unknown_vars.push_back(&_Tfric.get(0));
   init_unknown_vars.push_back(&_Tfric.get(1));
   init_unknown_vars.push_back(&_Tred.get(0));
   init_unknown_vars.push_back(&_Tred.get(1));
   init_unknown_vars.push_back(&_JqInv.get(0,0));
   init_unknown_vars.push_back(&_JqInv.get(0,1));
   init_unknown_vars.push_back(&_JqInv.get(1,0));
   init_unknown_vars.push_back(&_JqInv.get(1,1));
   init_unknown_vars.push_back(&_error);
   init_unknown_vars.push_back(&_q2_sample);
   init_unknown_vars.push_back(&_q1_sample);
   init_unknown_vars.push_back(&_dq2);
   init_unknown_vars.push_back(&_dq1);
   init_unknown_vars.push_back(&_dz);
   init_unknown_vars.push_back(&_dx);
   init_unknown_vars.push_back(&_q2);
   init_unknown_vars.push_back(&_q1);
   init_unknown_vars.push_back(&_z);
   init_unknown_vars.push_back(&_x);
   init_unknown_vars.push_back(&_zd);
   init_unknown_vars.push_back(&_xd);
   init_unknown_vars.push_back(&_DER_dq1);
   init_unknown_vars.push_back(&_DER_dx);
   init_unknown_vars.push_back(&_DER_dq2);
   init_unknown_vars.push_back(&_DER_dz);
   init_unknown_vars.push_back(&_$TMP_33.get(0));
   init_unknown_vars.push_back(&_$TMP_33.get(1));
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

 void Robot::clear_event_flags()
 {
     for (int i = 0; i < numRelations(); i++) zc[i] = -1;
     for (int i = 0; i < numMathEvents(); i++)
         if (eventFuncs[i] != NULL) eventFuncs[i]->setInit(true);
 }
 
 void Robot::init(double* q)
 {
     atInit = true;
     atEvent = false;
     timeValue = q[numVars()-1] = 0.0;
     clear_event_flags();
     alloc_array(_T,1,2);alloc_array(_PRE_T,1,2);
               
     alloc_array(_$STATESET1_x,1,2);alloc_array(_PRE_$STATESET1_x,1,2);
               alloc_array(_$STATESET2_x,1,2);alloc_array(_PRE_$STATESET2_x,1,2);
               
     alloc_array(_M,1,2);alloc_array(_PRE_M,1,2);
               alloc_array(_JxT,2,2,2);alloc_array(_PRE_JxT,2,2,2);
               alloc_array(_Ttplate,1,2);alloc_array(_PRE_Ttplate,1,2);
               alloc_array(_Tfarm,1,2);alloc_array(_PRE_Tfarm,1,2);
               alloc_array(_Tarm,1,2);alloc_array(_PRE_Tarm,1,2);
               alloc_array(_Tfric,1,2);alloc_array(_PRE_Tfric,1,2);
               alloc_array(_Tred,1,2);alloc_array(_PRE_Tred,1,2);
               alloc_array(_JqInv,2,2,2);alloc_array(_PRE_JqInv,2,2,2);
               alloc_array(_$TMP_33,1,2);alloc_array(_PRE_$TMP_33,1,2);
               
     alloc_array(_$STATESET2_A,2,2,4);alloc_array(_PRE_$STATESET2_A,2,2,4);
               alloc_array(_$STATESET1_A,2,2,4);alloc_array(_PRE_$STATESET1_A,2,2,4);
               
     alloc_array(_DER_$STATESET1_x,1,2);alloc_array(_PRE_DER_$STATESET1_x,1,2);
               alloc_array(_DER_$STATESET2_x,1,2);alloc_array(_PRE_DER_$STATESET2_x,1,2);
               
     // Get initial values as given in the model
   modelica_real tmp10;
   modelica_real tmp11;
   modelica_real tmp12;
   modelica_real tmp13;
   modelica_real tmp14;
   modelica_real tmp15;
   modelica_real tmp16;
   modelica_real tmp17;
   modelica_real tmp18;
   modelica_real tmp19;
   modelica_real tmp20;
   modelica_real tmp21;
   modelica_real tmp22;
   modelica_real tmp23;
   modelica_real tmp24;
   modelica_real tmp25;
   modelica_real tmp26;
   modelica_real tmp27;
   modelica_real tmp28;
   modelica_real tmp29;
   modelica_real tmp30;
   modelica_real tmp31;
   modelica_real tmp32;
   modelica_real tmp33;
   modelica_real tmp34;
   modelica_real tmp35;
   modelica_real tmp36;
   modelica_real tmp37;
   modelica_real tmp38;
   modelica_real tmp39;
   modelica_real tmp40;
   modelica_real tmp41;
     for (int jj = 0; jj < 4; jj++)
         colSelect_$STATESET1_A[jj] = jj;
     for (int jj = 0; jj < 2; jj++)
         rowSelect_$STATESET1_A[jj] = jj;
     for (int jj = 0; jj < 4; jj++)
         colSelect_$STATESET2_A[jj] = jj;
     for (int jj = 0; jj < 2; jj++)
         rowSelect_$STATESET2_A[jj] = jj;
             _T.get(0)=0.0;//_T.get(0)
             _T.get(1)=0.0;//_T.get(1)
            _pi=3.141592653589793;//_pi
            _xp=0.1;//_xp
            _L=0.3;//_L
            _l=0.7;//_l
            _m1=0.82;//_m1
            _m2=0.14;//_m2
            _m3=0.5;//_m3
            _ml=5.0;//_ml
            _Jmot=3.7e-05;//_Jmot
            _Jred=0.000909;//_Jred
            _I=0.018895002;//_I
            _Fs=3.0;//_Fs
            _Fv=0.5;//_Fv
            _nu=5.0;//_nu
            _g=9.81;//_g
            _sampleFreq=1000.0;//_sampleFreq
     tmp10 = $__start(_dz);
     tmp11 = $__start(_dq2);
     tmp12 = $__start(_dx);
     tmp13 = $__start(_dq1);
            _$STATESET1_x.get(0)=((((modelica_real)(modelica_integer)_$STATESET1_A.get(0,0)) * tmp10) + ((((modelica_real)(modelica_integer)_$STATESET1_A.get(0,1)) * tmp11) + ((((modelica_real)(modelica_integer)_$STATESET1_A.get(0,2)) * tmp12) + (((modelica_real)(modelica_integer)_$STATESET1_A.get(0,3)) * tmp13))));//_$STATESET1_x.get(0)
     tmp14 = $__start(_dz);
     tmp15 = $__start(_dq2);
     tmp16 = $__start(_dx);
     tmp17 = $__start(_dq1);
            _$STATESET1_x.get(1)=((((modelica_real)(modelica_integer)_$STATESET1_A.get(1,0)) * tmp14) + ((((modelica_real)(modelica_integer)_$STATESET1_A.get(1,1)) * tmp15) + ((((modelica_real)(modelica_integer)_$STATESET1_A.get(1,2)) * tmp16) + (((modelica_real)(modelica_integer)_$STATESET1_A.get(1,3)) * tmp17))));//_$STATESET1_x.get(1)
     tmp18 = $__start(_z);
     tmp19 = $__start(_q1);
     tmp20 = $__start(_x);
     tmp21 = $__start(_q2);
            _$STATESET2_x.get(0)=((((modelica_real)(modelica_integer)_$STATESET2_A.get(0,0)) * tmp18) + ((((modelica_real)(modelica_integer)_$STATESET2_A.get(0,1)) * tmp19) + ((((modelica_real)(modelica_integer)_$STATESET2_A.get(0,2)) * tmp20) + (((modelica_real)(modelica_integer)_$STATESET2_A.get(0,3)) * tmp21))));//_$STATESET2_x.get(0)
     tmp22 = $__start(_z);
     tmp23 = $__start(_q1);
     tmp24 = $__start(_x);
     tmp25 = $__start(_q2);
            _$STATESET2_x.get(1)=((((modelica_real)(modelica_integer)_$STATESET2_A.get(1,0)) * tmp22) + ((((modelica_real)(modelica_integer)_$STATESET2_A.get(1,1)) * tmp23) + ((((modelica_real)(modelica_integer)_$STATESET2_A.get(1,2)) * tmp24) + (((modelica_real)(modelica_integer)_$STATESET2_A.get(1,3)) * tmp25))));//_$STATESET2_x.get(1)
             _M.get(0)=0.0;
             _M.get(1)=0.0;
             _JxT.get(0,0)=0.0;
             _JxT.get(0,1)=0.0;
             _JxT.get(1,0)=0.0;
             _JxT.get(1,1)=0.0;
             _Ttplate.get(0)=0.0;
             _Ttplate.get(1)=0.0;
             _Tfarm.get(0)=0.0;
             _Tfarm.get(1)=0.0;
             _Tarm.get(0)=0.0;
             _Tarm.get(1)=0.0;
             _Tfric.get(0)=0.0;
             _Tfric.get(1)=0.0;
             _Tred.get(0)=0.0;
             _Tred.get(1)=0.0;
             _JqInv.get(0,0)=0.0;
             _JqInv.get(0,1)=0.0;
             _JqInv.get(1,0)=0.0;
             _JqInv.get(1,1)=0.0;
             _error=0.0;
             _q2_sample=0.0;
             _q1_sample=0.0;
             _dq2=0.0;
             _dq1=0.0;
             _dz=0.0;
             _dx=0.0;
            _q2=0.0818715;//_q2
            _q1=0.08384030000000001;//_q1
            _z=-0.6;//_z
             _x=0.0;//_x
             _zd=0.0;//_zd
             _xd=0.0;//_xd
             _DER_dq1=0.0;
             _DER_dx=0.0;
             _DER_dq2=0.0;
             _DER_dz=0.0;
             _$TMP_33.get(0)=0.0;
             _$TMP_33.get(1)=0.0;
            _$STATESET2_A.get(0,0)=(modelica_integer) 1;//_$STATESET2_A.get(0,0)
            _$STATESET2_A.get(1,0)=(modelica_integer) 0;//_$STATESET2_A.get(1,0)
            _$STATESET2_A.get(0,1)=(modelica_integer) 0;//_$STATESET2_A.get(0,1)
            _$STATESET2_A.get(1,1)=(modelica_integer) 1;//_$STATESET2_A.get(1,1)
            _$STATESET2_A.get(0,2)=(modelica_integer) 0;//_$STATESET2_A.get(0,2)
            _$STATESET2_A.get(1,2)=(modelica_integer) 0;//_$STATESET2_A.get(1,2)
            _$STATESET2_A.get(0,3)=(modelica_integer) 0;//_$STATESET2_A.get(0,3)
            _$STATESET2_A.get(1,3)=(modelica_integer) 0;//_$STATESET2_A.get(1,3)
            _$STATESET1_A.get(0,0)=(modelica_integer) 1;//_$STATESET1_A.get(0,0)
            _$STATESET1_A.get(1,0)=(modelica_integer) 0;//_$STATESET1_A.get(1,0)
            _$STATESET1_A.get(0,1)=(modelica_integer) 0;//_$STATESET1_A.get(0,1)
            _$STATESET1_A.get(1,1)=(modelica_integer) 1;//_$STATESET1_A.get(1,1)
            _$STATESET1_A.get(0,2)=(modelica_integer) 0;//_$STATESET1_A.get(0,2)
            _$STATESET1_A.get(1,2)=(modelica_integer) 0;//_$STATESET1_A.get(1,2)
            _$STATESET1_A.get(0,3)=(modelica_integer) 0;//_$STATESET1_A.get(0,3)
            _$STATESET1_A.get(1,3)=(modelica_integer) 0;//_$STATESET1_A.get(1,3)
            _sampleNumber=(modelica_integer) 0;//_sampleNumber
             _DER_$STATESET1_x.get(0)=0.0;
             _DER_$STATESET1_x.get(1)=0.0;
             _DER_$STATESET2_x.get(0)=0.0;
             _DER_$STATESET2_x.get(1)=0.0;
     // Save these to the old values so that pre() and edge() work
     save_vars();
     // Calculate any equations that provide initial values
     tmp26 = $__start(_dz);
     tmp27 = $__start(_dq2);
     tmp28 = $__start(_dx);
     tmp29 = $__start(_dq1);
     _$STATESET1_x.get(0) = ((((modelica_real)(modelica_integer)_$STATESET1_A.get(0,0)) * tmp26) + ((((modelica_real)(modelica_integer)_$STATESET1_A.get(0,1)) * tmp27) + ((((modelica_real)(modelica_integer)_$STATESET1_A.get(0,2)) * tmp28) + (((modelica_real)(modelica_integer)_$STATESET1_A.get(0,3)) * tmp29)))); 
     tmp30 = $__start(_dz);
     tmp31 = $__start(_dq2);
     tmp32 = $__start(_dx);
     tmp33 = $__start(_dq1);
     _$STATESET1_x.get(1) = ((((modelica_real)(modelica_integer)_$STATESET1_A.get(1,0)) * tmp30) + ((((modelica_real)(modelica_integer)_$STATESET1_A.get(1,1)) * tmp31) + ((((modelica_real)(modelica_integer)_$STATESET1_A.get(1,2)) * tmp32) + (((modelica_real)(modelica_integer)_$STATESET1_A.get(1,3)) * tmp33)))); 
     tmp34 = $__start(_z);
     tmp35 = $__start(_q1);
     tmp36 = $__start(_x);
     tmp37 = $__start(_q2);
     _$STATESET2_x.get(0) = ((((modelica_real)(modelica_integer)_$STATESET2_A.get(0,0)) * tmp34) + ((((modelica_real)(modelica_integer)_$STATESET2_A.get(0,1)) * tmp35) + ((((modelica_real)(modelica_integer)_$STATESET2_A.get(0,2)) * tmp36) + (((modelica_real)(modelica_integer)_$STATESET2_A.get(0,3)) * tmp37)))); 
     tmp38 = $__start(_z);
     tmp39 = $__start(_q1);
     tmp40 = $__start(_x);
     tmp41 = $__start(_q2);
     _$STATESET2_x.get(1) = ((((modelica_real)(modelica_integer)_$STATESET2_A.get(1,0)) * tmp38) + ((((modelica_real)(modelica_integer)_$STATESET2_A.get(1,1)) * tmp39) + ((((modelica_real)(modelica_integer)_$STATESET2_A.get(1,2)) * tmp40) + (((modelica_real)(modelica_integer)_$STATESET2_A.get(1,3)) * tmp41)))); 
     bound_params();
     // Solve for any remaining unknowns
     solve_for_initial_unknowns();
     selectStateVars();
     calc_vars();
     save_vars();
     q[0]=_$STATESET1_x.get(0);
     q[1]=_$STATESET1_x.get(1);
     q[2]=_$STATESET2_x.get(0);
     q[3]=_$STATESET2_x.get(1);
     atInit = false;
     for (int i = 0; i < numMathEvents(); i++)
         if (eventFuncs[i] != NULL) eventFuncs[i]->setInit(false);
 }

 void Robot::der_func(const double* q, double* dq)
 {
     calc_vars(q);
     dq[0]=_DER_$STATESET1_x.get(0);
     dq[1]=_DER_$STATESET1_x.get(1);
     dq[2]=_DER_$STATESET2_x.get(0);
     dq[3]=_DER_$STATESET2_x.get(1);
     dq[numVars()-1] = 1.0;
     restore_vars();
 }

 void Robot::postStep(double* q)
 {
     calc_vars(q);
     if (selectStateVars())
     {
         q[0] = _$STATESET1_x.get(0);
         q[1] = _$STATESET1_x.get(1);
         q[2] = _$STATESET2_x.get(0);
         q[3] = _$STATESET2_x.get(1);
         calc_vars(q,true);
     }
     save_vars();
 }

 void Robot::save_vars()
 {
   _PRE_timeValue = timeValue;
   _PRE_$STATESET1_x.get(0)=_$STATESET1_x.get(0);
   _PRE_$STATESET1_x.get(1)=_$STATESET1_x.get(1);
   _PRE_$STATESET2_x.get(0)=_$STATESET2_x.get(0);
   _PRE_$STATESET2_x.get(1)=_$STATESET2_x.get(1);
   _PRE_DER_$STATESET1_x.get(0)=_DER_$STATESET1_x.get(0);
   _PRE_DER_$STATESET1_x.get(1)=_DER_$STATESET1_x.get(1);
   _PRE_DER_$STATESET2_x.get(0)=_DER_$STATESET2_x.get(0);
   _PRE_DER_$STATESET2_x.get(1)=_DER_$STATESET2_x.get(1);
   _PRE_M.get(0)=_M.get(0);
   _PRE_M.get(1)=_M.get(1);
   _PRE_JxT.get(0,0)=_JxT.get(0,0);
   _PRE_JxT.get(0,1)=_JxT.get(0,1);
   _PRE_JxT.get(1,0)=_JxT.get(1,0);
   _PRE_JxT.get(1,1)=_JxT.get(1,1);
   _PRE_Ttplate.get(0)=_Ttplate.get(0);
   _PRE_Ttplate.get(1)=_Ttplate.get(1);
   _PRE_Tfarm.get(0)=_Tfarm.get(0);
   _PRE_Tfarm.get(1)=_Tfarm.get(1);
   _PRE_Tarm.get(0)=_Tarm.get(0);
   _PRE_Tarm.get(1)=_Tarm.get(1);
   _PRE_Tfric.get(0)=_Tfric.get(0);
   _PRE_Tfric.get(1)=_Tfric.get(1);
   _PRE_Tred.get(0)=_Tred.get(0);
   _PRE_Tred.get(1)=_Tred.get(1);
   _PRE_JqInv.get(0,0)=_JqInv.get(0,0);
   _PRE_JqInv.get(0,1)=_JqInv.get(0,1);
   _PRE_JqInv.get(1,0)=_JqInv.get(1,0);
   _PRE_JqInv.get(1,1)=_JqInv.get(1,1);
   _PRE_error=_error;
   _PRE_q2_sample=_q2_sample;
   _PRE_q1_sample=_q1_sample;
   _PRE_dq2=_dq2;
   _PRE_dq1=_dq1;
   _PRE_dz=_dz;
   _PRE_dx=_dx;
   _PRE_q2=_q2;
   _PRE_q1=_q1;
   _PRE_z=_z;
   _PRE_x=_x;
   _PRE_zd=_zd;
   _PRE_xd=_xd;
   _PRE_DER_dq1=_DER_dq1;
   _PRE_DER_dx=_DER_dx;
   _PRE_DER_dq2=_DER_dq2;
   _PRE_DER_dz=_DER_dz;
   _PRE_$TMP_33.get(0)=_$TMP_33.get(0);
   _PRE_$TMP_33.get(1)=_$TMP_33.get(1);
   _PRE_$STATESET2_A.get(0,0)=_$STATESET2_A.get(0,0);
   _PRE_$STATESET2_A.get(1,0)=_$STATESET2_A.get(1,0);
   _PRE_$STATESET2_A.get(0,1)=_$STATESET2_A.get(0,1);
   _PRE_$STATESET2_A.get(1,1)=_$STATESET2_A.get(1,1);
   _PRE_$STATESET2_A.get(0,2)=_$STATESET2_A.get(0,2);
   _PRE_$STATESET2_A.get(1,2)=_$STATESET2_A.get(1,2);
   _PRE_$STATESET2_A.get(0,3)=_$STATESET2_A.get(0,3);
   _PRE_$STATESET2_A.get(1,3)=_$STATESET2_A.get(1,3);
   _PRE_$STATESET1_A.get(0,0)=_$STATESET1_A.get(0,0);
   _PRE_$STATESET1_A.get(1,0)=_$STATESET1_A.get(1,0);
   _PRE_$STATESET1_A.get(0,1)=_$STATESET1_A.get(0,1);
   _PRE_$STATESET1_A.get(1,1)=_$STATESET1_A.get(1,1);
   _PRE_$STATESET1_A.get(0,2)=_$STATESET1_A.get(0,2);
   _PRE_$STATESET1_A.get(1,2)=_$STATESET1_A.get(1,2);
   _PRE_$STATESET1_A.get(0,3)=_$STATESET1_A.get(0,3);
   _PRE_$STATESET1_A.get(1,3)=_$STATESET1_A.get(1,3);
   _PRE_sampleNumber=_sampleNumber;
   _PRE_DER_q1=_DER_q1;
   _PRE_DER_q2=_DER_q2;
   _PRE_DER_x=_DER_x;
   _PRE_DER_z=_DER_z;
   _PRE_T.get(0)=_T.get(0);
   _PRE_T.get(1)=_T.get(1);
   _PRE_pi=_pi;
   _PRE_xp=_xp;
   _PRE_L=_L;
   _PRE_l=_l;
   _PRE_m1=_m1;
   _PRE_m2=_m2;
   _PRE_m3=_m3;
   _PRE_ml=_ml;
   _PRE_Jmot=_Jmot;
   _PRE_Jred=_Jred;
   _PRE_I=_I;
   _PRE_Fs=_Fs;
   _PRE_Fv=_Fv;
   _PRE_nu=_nu;
   _PRE_g=_g;
   _PRE_sampleFreq=_sampleFreq;
 }

 void Robot::restore_vars()
 {
   timeValue = _PRE_timeValue;
   _$STATESET1_x.get(0)=_PRE_$STATESET1_x.get(0);
   _$STATESET1_x.get(1)=_PRE_$STATESET1_x.get(1);
   _$STATESET2_x.get(0)=_PRE_$STATESET2_x.get(0);
   _$STATESET2_x.get(1)=_PRE_$STATESET2_x.get(1);
   _DER_$STATESET1_x.get(0)=_PRE_DER_$STATESET1_x.get(0);
   _DER_$STATESET1_x.get(1)=_PRE_DER_$STATESET1_x.get(1);
   _DER_$STATESET2_x.get(0)=_PRE_DER_$STATESET2_x.get(0);
   _DER_$STATESET2_x.get(1)=_PRE_DER_$STATESET2_x.get(1);
   _M.get(0)=_PRE_M.get(0);
   _M.get(1)=_PRE_M.get(1);
   _JxT.get(0,0)=_PRE_JxT.get(0,0);
   _JxT.get(0,1)=_PRE_JxT.get(0,1);
   _JxT.get(1,0)=_PRE_JxT.get(1,0);
   _JxT.get(1,1)=_PRE_JxT.get(1,1);
   _Ttplate.get(0)=_PRE_Ttplate.get(0);
   _Ttplate.get(1)=_PRE_Ttplate.get(1);
   _Tfarm.get(0)=_PRE_Tfarm.get(0);
   _Tfarm.get(1)=_PRE_Tfarm.get(1);
   _Tarm.get(0)=_PRE_Tarm.get(0);
   _Tarm.get(1)=_PRE_Tarm.get(1);
   _Tfric.get(0)=_PRE_Tfric.get(0);
   _Tfric.get(1)=_PRE_Tfric.get(1);
   _Tred.get(0)=_PRE_Tred.get(0);
   _Tred.get(1)=_PRE_Tred.get(1);
   _JqInv.get(0,0)=_PRE_JqInv.get(0,0);
   _JqInv.get(0,1)=_PRE_JqInv.get(0,1);
   _JqInv.get(1,0)=_PRE_JqInv.get(1,0);
   _JqInv.get(1,1)=_PRE_JqInv.get(1,1);
   _error=_PRE_error;
   _q2_sample=_PRE_q2_sample;
   _q1_sample=_PRE_q1_sample;
   _dq2=_PRE_dq2;
   _dq1=_PRE_dq1;
   _dz=_PRE_dz;
   _dx=_PRE_dx;
   _q2=_PRE_q2;
   _q1=_PRE_q1;
   _z=_PRE_z;
   _x=_PRE_x;
   _zd=_PRE_zd;
   _xd=_PRE_xd;
   _DER_dq1=_PRE_DER_dq1;
   _DER_dx=_PRE_DER_dx;
   _DER_dq2=_PRE_DER_dq2;
   _DER_dz=_PRE_DER_dz;
   _$TMP_33.get(0)=_PRE_$TMP_33.get(0);
   _$TMP_33.get(1)=_PRE_$TMP_33.get(1);
   _$STATESET2_A.get(0,0)=_PRE_$STATESET2_A.get(0,0);
   _$STATESET2_A.get(1,0)=_PRE_$STATESET2_A.get(1,0);
   _$STATESET2_A.get(0,1)=_PRE_$STATESET2_A.get(0,1);
   _$STATESET2_A.get(1,1)=_PRE_$STATESET2_A.get(1,1);
   _$STATESET2_A.get(0,2)=_PRE_$STATESET2_A.get(0,2);
   _$STATESET2_A.get(1,2)=_PRE_$STATESET2_A.get(1,2);
   _$STATESET2_A.get(0,3)=_PRE_$STATESET2_A.get(0,3);
   _$STATESET2_A.get(1,3)=_PRE_$STATESET2_A.get(1,3);
   _$STATESET1_A.get(0,0)=_PRE_$STATESET1_A.get(0,0);
   _$STATESET1_A.get(1,0)=_PRE_$STATESET1_A.get(1,0);
   _$STATESET1_A.get(0,1)=_PRE_$STATESET1_A.get(0,1);
   _$STATESET1_A.get(1,1)=_PRE_$STATESET1_A.get(1,1);
   _$STATESET1_A.get(0,2)=_PRE_$STATESET1_A.get(0,2);
   _$STATESET1_A.get(1,2)=_PRE_$STATESET1_A.get(1,2);
   _$STATESET1_A.get(0,3)=_PRE_$STATESET1_A.get(0,3);
   _$STATESET1_A.get(1,3)=_PRE_$STATESET1_A.get(1,3);
   _sampleNumber=_PRE_sampleNumber;
   _DER_q1=_PRE_DER_q1;
   _DER_q2=_PRE_DER_q2;
   _DER_x=_PRE_DER_x;
   _DER_z=_PRE_DER_z;
     _T.get(0)=_PRE_T.get(0);
     _T.get(1)=_PRE_T.get(1);
     _pi=_PRE_pi;
     _xp=_PRE_xp;
     _L=_PRE_L;
     _l=_PRE_l;
     _m1=_PRE_m1;
     _m2=_PRE_m2;
     _m3=_PRE_m3;
     _ml=_PRE_ml;
     _Jmot=_PRE_Jmot;
     _Jred=_PRE_Jred;
     _I=_PRE_I;
     _Fs=_PRE_Fs;
     _Fv=_PRE_Fv;
     _nu=_PRE_nu;
     _g=_PRE_g;
     _sampleFreq=_PRE_sampleFreq;
 }

 void Robot::calc_vars(const double* q, bool doReinit)
 {
     bool reInit = false;
     active_model = this;
     if (atEvent || doReinit) clear_event_flags();
     // Copy state variable arrays to values used in the odes
     if (q != NULL)
     {
         timeValue = q[numVars()-1];
         _$STATESET1_x.get(0)=q[0];
         _$STATESET1_x.get(1)=q[1];
         _$STATESET2_x.get(0)=q[2];
         _$STATESET2_x.get(1)=q[3];
     }
     modelica_real tmp42;
     modelica_real tmp43;
     modelica_real tmp44;
     modelica_real tmp45;
     modelica_real tmp46;
     modelica_real tmp47;
     modelica_boolean tmp48;
     modelica_boolean tmp49;
     modelica_real tmp51;
     modelica_real tmp52;
     modelica_real tmp53;
     modelica_real tmp54;
     modelica_real tmp55;
     modelica_real tmp56;
     modelica_real tmp57;
     modelica_real tmp58;
     modelica_real tmp59;
     modelica_real tmp60;
     modelica_real tmp61;
     modelica_real tmp62;
     modelica_integer tmp63;
     modelica_integer tmp64;
     modelica_real tmp65;
     modelica_real tmp66;
     modelica_real tmp67;
     modelica_real tmp68;
     modelica_real tmp69;
     modelica_real tmp70;
     modelica_real tmp71;
     modelica_real tmp72;
     modelica_real tmp73;
     modelica_real tmp74;
     modelica_real tmp75;
     modelica_real tmp76;
     modelica_real tmp78;
     modelica_real tmp79;
     modelica_real tmp80;
     modelica_real tmp81;
     modelica_real tmp82;
     modelica_real tmp83;
     modelica_real tmp84;
     modelica_real tmp85;
     modelica_real tmp86;
     modelica_real tmp87;
     modelica_real tmp88;
     modelica_real tmp89;
     modelica_real tmp90;
     modelica_real tmp91;
     modelica_real tmp92;
     modelica_real tmp93;
     modelica_real tmp94;
     modelica_real tmp95;
     modelica_real tmp96;
     modelica_real tmp97;
     modelica_real tmp98;
     modelica_real tmp99;
     modelica_real tmp100;
     modelica_real tmp101;
     modelica_real tmp102;
     modelica_real tmp103;
     modelica_real tmp104;
     modelica_real tmp105;
     modelica_real tmp106;
     modelica_real tmp107;
     modelica_real tmp108;
     modelica_real tmp109;
     modelica_real tmp110;
     modelica_real tmp111;
     modelica_real tmp112;
     modelica_real tmp113;
     modelica_real tmp114;
     modelica_real tmp115;
     modelica_real tmp116;
     modelica_real tmp117;
     // Primary equations
     tmp42 = cos((0.5 * (_pi * timeValue)));
     _zd = (-0.7 + (0.1 * tmp42)); 
     tmp43 = sin((0.5 * (_pi * timeValue)));
     _xd = (-0.35 * tmp43); 
     solve_residualFunc38_cpp();
     tmp44 = cos(_q2);
     _JxT.get(0,1) = (2.0 * (_x + (_xp + (_L * tmp44)))); 
     tmp45 = cos(_q1);
     _JxT.get(0,0) = (2.0 * (_x + ((-_xp) - (_L * tmp45)))); 
     /*#modelicaLine [:0:0-0:0]*/
     _sampleNumber = _PRE_sampleNumber;
     /*#endModelicaLine*/
     /*#modelicaLine [:0:0-0:0]*/
     tmp46 = $__start(_q2_sample);
     _q2_sample = tmp46;
     /*#endModelicaLine*/
     /*#modelicaLine [:0:0-0:0]*/
     tmp47 = $__start(_q1_sample);
     _q1_sample = tmp47;
     /*#endModelicaLine*/
     /*#modelicaLine [/home/nutarojj/Code/adevs-code/examples/modelica/HighIndexWithControl/SampledRobot.mo:4:3-8:9]*/
     tmp48 = initial();
     tmp49 = sample((modelica_integer) 1, 0.0, (1.0 / _sampleFreq));
     if ((tmp48 || tmp49)) {
       /*#modelicaLine [/home/nutarojj/Code/adevs-code/examples/modelica/HighIndexWithControl/SampledRobot.mo:5:5-5:20]*/
       _q1_sample = _q1;
       /*#endModelicaLine*/
       /*#modelicaLine [/home/nutarojj/Code/adevs-code/examples/modelica/HighIndexWithControl/SampledRobot.mo:6:5-6:20]*/
       _q2_sample = _q2;
       /*#endModelicaLine*/
       /*#modelicaLine [/home/nutarojj/Code/adevs-code/examples/modelica/HighIndexWithControl/SampledRobot.mo:7:2-7:32]*/
       _sampleNumber = ((modelica_integer) 1 + (modelica_integer)_sampleNumber);
       /*#endModelicaLine*/
     }
     /*#endModelicaLine*/
     double* A50 = new double[4*4];
     double* b50 = new double[4];
     long int* p50 = new long int[4];
     for (int i = 0; i < 4; i++)
     {
       for (int j = 0; j < 4; j++)
       {
         A50[j+i*4] = 0.0;
       }
       p50[i] = i;
       b50[i] = 0.0;
     }
     tmp51 = sin(_q2);
     A50[0+1*4] = (-2.0 * (_z + (_L * tmp51)));
     tmp52 = cos(_q2);
     A50[0+2*4] = (-2.0 * (_x + (_xp + (_L * tmp52))));
     tmp53 = cos(_q2);
     tmp54 = sin(_q2);
     tmp55 = sin(_q2);
     tmp56 = cos(_q2);
     A50[0+3*4] = (-2.0 * (((_x + (_xp + (_L * tmp53))) * ((-_L) * tmp54)) + ((_z + (_L * tmp55)) * (_L * tmp56))));
     A50[1+0*4] = (-((modelica_real)(modelica_integer)_$STATESET1_A.get(0,3)));
     A50[1+1*4] = (-((modelica_real)(modelica_integer)_$STATESET1_A.get(0,0)));
     A50[1+2*4] = (-((modelica_real)(modelica_integer)_$STATESET1_A.get(0,2)));
     A50[1+3*4] = (-((modelica_real)(modelica_integer)_$STATESET1_A.get(0,1)));
     A50[2+0*4] = (-((modelica_real)(modelica_integer)_$STATESET1_A.get(1,3)));
     A50[2+1*4] = (-((modelica_real)(modelica_integer)_$STATESET1_A.get(1,0)));
     A50[2+2*4] = (-((modelica_real)(modelica_integer)_$STATESET1_A.get(1,2)));
     A50[2+3*4] = (-((modelica_real)(modelica_integer)_$STATESET1_A.get(1,1)));
     tmp57 = cos(_q1);
     tmp58 = sin(_q1);
     tmp59 = sin(_q1);
     tmp60 = cos(_q1);
     A50[3+0*4] = (-2.0 * ((((_L * tmp57) + (_xp - _x)) * ((-_L) * tmp58)) + ((_z + (_L * tmp59)) * (_L * tmp60))));
     tmp61 = sin(_q1);
     A50[3+1*4] = (-2.0 * (_z + (_L * tmp61)));
     tmp62 = cos(_q1);
     A50[3+2*4] = (-2.0 * (_x + ((-_xp) - (_L * tmp62))));
     b50[0] = 0.0;
     b50[1] = (-_$STATESET1_x.get(0));
     b50[2] = (-_$STATESET1_x.get(1));
     b50[3] = 0.0;
     GETRF(A50,4,p50);
     GETRS(A50,4,p50,b50);
     _dq1 = b50[0];
     _dz = b50[1];
     _dx = b50[2];
     _dq2 = b50[3];
     delete [] A50;
     delete [] b50;
     delete [] p50;
     tmp63 = sign(_dq1);
     _Tfric.get(0) = ((((modelica_real)tmp63) * _Fs) + (_Fv * _dq1)); 
     _DER_$STATESET2_x.get(0) = ((((modelica_real)(modelica_integer)_$STATESET2_A.get(0,0)) * _dz) + ((((modelica_real)(modelica_integer)_$STATESET2_A.get(0,1)) * _dq1) + ((((modelica_real)(modelica_integer)_$STATESET2_A.get(0,2)) * _dx) + (((modelica_real)(modelica_integer)_$STATESET2_A.get(0,3)) * _dq2)))); 
     _DER_$STATESET2_x.get(1) = ((((modelica_real)(modelica_integer)_$STATESET2_A.get(1,0)) * _dz) + ((((modelica_real)(modelica_integer)_$STATESET2_A.get(1,1)) * _dq1) + ((((modelica_real)(modelica_integer)_$STATESET2_A.get(1,2)) * _dx) + (((modelica_real)(modelica_integer)_$STATESET2_A.get(1,3)) * _dq2)))); 
     tmp64 = sign(_dq2);
     _Tfric.get(1) = ((((modelica_real)tmp64) * _Fs) + (_Fv * _dq2)); 
     tmp65 = sin(_q1);
     _JxT.get(1,0) = (2.0 * (_z + (_L * tmp65))); 
     tmp66 = sin(_q2);
     _JxT.get(1,1) = (2.0 * (_z + (_L * tmp66))); 
     tmp67 = cos(_q1);
     tmp68 = sin(_q1);
     tmp69 = sin(_q1);
     tmp70 = cos(_q1);
     tmp71 = DIVISION(1.0, ((((_x + ((-_xp) - (_L * tmp67))) * (_L * tmp68)) + ((_z + (_L * tmp69)) * (_L * tmp70))) * 2.0));
     _JqInv.get(0,0) = tmp71; 
     tmp72 = cos(_q2);
     tmp73 = sin(_q2);
     tmp74 = sin(_q2);
     tmp75 = cos(_q2);
     tmp76 = DIVISION(1.0, ((((_x + (_xp + (_L * tmp72))) * ((-_L) * tmp73)) + ((_z + (_L * tmp74)) * (_L * tmp75))) * 2.0));
     _JqInv.get(1,1) = tmp76; 
     double* A77 = new double[14*14];
     double* b77 = new double[14];
     long int* p77 = new long int[14];
     for (int i = 0; i < 14; i++)
     {
       for (int j = 0; j < 14; j++)
       {
         A77[j+i*14] = 0.0;
       }
       p77[i] = i;
       b77[i] = 0.0;
     }
     tmp78 = cos(_q2);
     tmp79 = sin(_q2);
     tmp80 = sin(_q2);
     tmp81 = cos(_q2);
     A77[0+2*14] = (-2.0 * (((_x + (_xp + (_L * tmp78))) * ((-_L) * tmp79)) + ((_z + (_L * tmp80)) * (_L * tmp81))));
     tmp82 = sin(_q2);
     A77[0+9*14] = (-2.0 * (_z + (_L * tmp82)));
     tmp83 = cos(_q2);
     A77[0+13*14] = (-2.0 * (_x + (_xp + (_L * tmp83))));
     A77[1+12*14] = -1.0;
     A77[1+13*14] = (((-_ml) - _m2) - _m3);
     A77[2+6*14] = ((-_JxT.get(0,0)) * _JqInv.get(0,0));
     A77[2+11*14] = ((-_JxT.get(0,1)) * _JqInv.get(1,1));
     A77[2+12*14] = 1.0;
     A77[3+6*14] = ((-_JxT.get(1,0)) * _JqInv.get(0,0));
     A77[3+10*14] = 1.0;
     A77[3+11*14] = ((-_JxT.get(1,1)) * _JqInv.get(1,1));
     A77[4+9*14] = (((-_ml) - _m2) - _m3);
     A77[4+10*14] = -1.0;
     tmp84 = cos(_q1);
     tmp85 = sin(_q1);
     tmp86 = sin(_q1);
     tmp87 = cos(_q1);
     A77[5+8*14] = (-2.0 * ((((_L * tmp84) + (_xp - _x)) * ((-_L) * tmp85)) + ((_z + (_L * tmp86)) * (_L * tmp87))));
     tmp88 = sin(_q1);
     A77[5+9*14] = (-2.0 * (_z + (_L * tmp88)));
     tmp89 = cos(_q1);
     A77[5+13*14] = (-2.0 * (_x + ((-_xp) - (_L * tmp89))));
     A77[6+7*14] = 1.0;
     tmp90 = _L;A77[6+8*14] = (-0.5 * (_m2 * (tmp90 * tmp90)));
     A77[7+4*14] = -1.0;
     A77[7+5*14] = -1.0;
     A77[7+6*14] = -1.0;
     A77[7+7*14] = -1.0;
     A77[8+5*14] = 1.0;
     A77[8+8*14] = (-_I);
     A77[9+4*14] = 1.0;
     tmp91 = _nu;A77[9+8*14] = ((-(tmp91 * tmp91)) * (_Jmot + _Jred));
     A77[10+0*14] = -1.0;
     A77[10+1*14] = -1.0;
     A77[10+3*14] = -1.0;
     A77[10+11*14] = -1.0;
     tmp92 = _L;A77[11+2*14] = (-0.5 * (_m2 * (tmp92 * tmp92)));
     A77[11+3*14] = 1.0;
     A77[12+1*14] = 1.0;
     A77[12+2*14] = (-_I);
     A77[13+0*14] = 1.0;
     tmp93 = _nu;A77[13+2*14] = ((-(tmp93 * tmp93)) * (_Jmot + _Jred));
     tmp94 = cos(_q2);
     tmp95 = cos(_q2);
     tmp96 = _dq2;tmp97 = sin(_q2);
     tmp98 = (_dx + (_L * ((-tmp97) * _dq2)));tmp99 = sin(_q2);
     tmp100 = sin(_q2);
     tmp101 = _dq2;tmp102 = cos(_q2);
     tmp103 = (_dz + (_L * (tmp102 * _dq2)));b77[0] = (2.0 * (((_x + (_xp + (_L * tmp94))) * (_L * ((-tmp95) * (tmp96 * tmp96)))) + ((tmp98 * tmp98) + (((_z + (_L * tmp99)) * (_L * ((-tmp100) * (tmp101 * tmp101)))) + (tmp103 * tmp103)))));
     b77[1] = 0.0;
     b77[2] = 0.0;
     b77[3] = 0.0;
     b77[4] = ((((-_ml) - _m2) - _m3) * _g);
     tmp104 = cos(_q1);
     tmp105 = cos(_q1);
     tmp106 = _dq1;tmp107 = sin(_q1);
     tmp108 = (_dx - (_L * ((-tmp107) * _dq1)));tmp109 = sin(_q1);
     tmp110 = sin(_q1);
     tmp111 = _dq1;tmp112 = cos(_q1);
     tmp113 = (_dz + (_L * (tmp112 * _dq1)));b77[5] = (2.0 * ((((_L * tmp104) + (_xp - _x)) * (_L * ((-tmp105) * (tmp106 * tmp106)))) + ((tmp108 * tmp108) + (((_z + (_L * tmp109)) * (_L * ((-tmp110) * (tmp111 * tmp111)))) + (tmp113 * tmp113)))));
     tmp114 = cos(_q1);
     b77[6] = (0.5 * (_m2 * ((-_L) * (_g * tmp114))));
     b77[7] = (_Tfric.get(0) - _T.get(0));
     tmp115 = cos(_q1);
     b77[8] = ((-_m1) * (_g * tmp115));
     b77[9] = 0.0;
     b77[10] = (_Tfric.get(1) - _T.get(1));
     tmp116 = cos(_q2);
     b77[11] = (0.5 * (_m2 * ((-_L) * (_g * tmp116))));
     tmp117 = cos(_q2);
     b77[12] = ((-_m1) * (_g * tmp117));
     b77[13] = 0.0;
     GETRF(A77,14,p77);
     GETRS(A77,14,p77,b77);
     _Tred.get(1) = b77[0];
     _Tarm.get(1) = b77[1];
     _DER_dq2 = b77[2];
     _Tfarm.get(1) = b77[3];
     _Tred.get(0) = b77[4];
     _Tarm.get(0) = b77[5];
     _Ttplate.get(0) = b77[6];
     _Tfarm.get(0) = b77[7];
     _DER_dq1 = b77[8];
     _DER_dz = b77[9];
     _M.get(1) = b77[10];
     _Ttplate.get(1) = b77[11];
     _M.get(0) = b77[12];
     _DER_dx = b77[13];
     delete [] A77;
     delete [] b77;
     delete [] p77;
     _DER_$STATESET1_x.assign((modelica_real)((((((modelica_real)(modelica_integer)_$STATESET1_A.get(0,0)) * _DER_dz) + (((modelica_real)(modelica_integer)_$STATESET1_A.get(0,1)) * _DER_dq2)) + (((modelica_real)(modelica_integer)_$STATESET1_A.get(0,2)) * _DER_dx)) + (((modelica_real)(modelica_integer)_$STATESET1_A.get(0,3)) * _DER_dq1)),(modelica_real)((((((modelica_real)(modelica_integer)_$STATESET1_A.get(1,0)) * _DER_dz) + (((modelica_real)(modelica_integer)_$STATESET1_A.get(1,1)) * _DER_dq2)) + (((modelica_real)(modelica_integer)_$STATESET1_A.get(1,2)) * _DER_dx)) + (((modelica_real)(modelica_integer)_$STATESET1_A.get(1,3)) * _DER_dq1)));
     _error = (fabs((_x - _xd)) + fabs((_z - _zd))); 
     // Alias equations
     // Reinits
     // Alias assignments
     _DER_q1 = _dq1;
     _DER_q2 = _dq2;
     _DER_x = _dx;
     _DER_z = _dz;
     if (atEvent && !reInit) reInit = check_for_new_events();
     if (reInit)
     {
         save_vars();
         calc_vars(NULL,reInit);
     }
 }

 
 bool Robot::check_for_new_events()
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
 
 void Robot::state_event_func(const double* q, double* z)
 {
     calc_vars(q);
     extra_state_event_funcs(&(z[numStateEvents()]));
     restore_vars();
 }
 
 bool Robot::sample(int index, double tStart, double tInterval)
 {
   index--;
   assert(index >= 0);
     if (samples[index] == NULL)
         samples[index] = new AdevsSampleData(tStart,tInterval);
     return samples[index]->atEvent(timeValue,epsilon);
 }
 
 double Robot::time_event_func(const double* q)
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
 
 void Robot::internal_event(double* q, const bool* state_event)
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
     q[0]=_$STATESET1_x.get(0);
     q[1]=_$STATESET1_x.get(1);
     q[2]=_$STATESET2_x.get(0);
     q[3]=_$STATESET2_x.get(1);
     for (int i = 0; i < numMathEvents(); i++)
         if (eventFuncs[i] != NULL) eventFuncs[i]->setInit(false);
     atEvent = false;
 }
 
 double Robot::floor(double expr, int index)
 {
     if (eventFuncs[index] == NULL)
         eventFuncs[index] = new AdevsFloorFunc(epsilon);
     return eventFuncs[index]->calcValue(expr);
 }
 
 double Robot::div(double x, double y, int index)
 {
     if (eventFuncs[index] == NULL)
         eventFuncs[index] = new AdevsDivFunc(epsilon);
     return eventFuncs[index]->calcValue(x/y);
 }
 
 int Robot::integer(double expr, int index)
 {
     if (eventFuncs[index] == NULL)
         eventFuncs[index] = new AdevsFloorFunc(epsilon);
     return int(eventFuncs[index]->calcValue(expr));
 }
 
 double Robot::ceil(double expr, int index)
 {
     if (eventFuncs[index] == NULL)
         eventFuncs[index] = new AdevsCeilFunc(epsilon);
     return eventFuncs[index]->calcValue(expr);
 }

 void Robot::calc_Jacobian_StateSetJac1()
 {
     double _S_[4];
     double &_dz$pDERStateSetJac1$Pdz = _S_[0];
     double &_dq2$pDERStateSetJac1$Pdq2 = _S_[1];
     double &_dx$pDERStateSetJac1$Pdx = _S_[2];
     double &_dq1$pDERStateSetJac1$Pdq1 = _S_[3];
     for (int ii = 0; ii < 4; ii++)
         _S_[ii] = 0.0;
     double _$STATESET1$PJ$lB2$rB$pDERStateSetJac1$PdummyVarStateSetJac1;
     double _$STATESET1$PJ$lB1$rB$pDERStateSetJac1$PdummyVarStateSetJac1;
     modelica_real tmp118;
     modelica_real tmp119;
     modelica_real tmp120;
     modelica_real tmp121;
     modelica_real tmp122;
     modelica_real tmp123;
     modelica_real tmp124;
     modelica_real tmp125;
     for (int col = 0; col < 4; col++)
     {
         _S_[col] = 1.0;
         tmp118 = cos(_q2);
         tmp119 = sin(_q2);
         tmp120 = sin(_q2);
         tmp121 = cos(_q2);
         _$STATESET1$PJ$lB2$rB$pDERStateSetJac1$PdummyVarStateSetJac1 = (-2.0 * (((_x + (_xp + (_L * tmp118))) * (_dx$pDERStateSetJac1$Pdx + (_L * ((-tmp119) * _dq2$pDERStateSetJac1$Pdq2)))) + ((_z + (_L * tmp120)) * (_dz$pDERStateSetJac1$Pdz + (_L * (tmp121 * _dq2$pDERStateSetJac1$Pdq2)))))); 
         tmp122 = cos(_q1);
         tmp123 = sin(_q1);
         tmp124 = sin(_q1);
         tmp125 = cos(_q1);
         _$STATESET1$PJ$lB1$rB$pDERStateSetJac1$PdummyVarStateSetJac1 = (-2.0 * (((_x + ((-_xp) - (_L * tmp122))) * (_dx$pDERStateSetJac1$Pdx - (_L * ((-tmp123) * _dq1$pDERStateSetJac1$Pdq1)))) + ((_z + (_L * tmp124)) * (_dz$pDERStateSetJac1$Pdz + (_L * (tmp125 * _dq1$pDERStateSetJac1$Pdq1)))))); 
         _S_[col] = 0.0;
         _Jacobian_StateSetJac1[0+2*col] = _$STATESET1$PJ$lB2$rB$pDERStateSetJac1$PdummyVarStateSetJac1;
         _Jacobian_StateSetJac1[1+2*col] = _$STATESET1$PJ$lB1$rB$pDERStateSetJac1$PdummyVarStateSetJac1;
     }
 }
 

 void Robot::calc_Jacobian_StateSetJac0()
 {
     double _S_[4];
     double &_z$pDERStateSetJac0$Pz = _S_[0];
     double &_q1$pDERStateSetJac0$Pq1 = _S_[1];
     double &_x$pDERStateSetJac0$Px = _S_[2];
     double &_q2$pDERStateSetJac0$Pq2 = _S_[3];
     for (int ii = 0; ii < 4; ii++)
         _S_[ii] = 0.0;
     double _$STATESET2$PJ$lB2$rB$pDERStateSetJac0$PdummyVarStateSetJac0;
     double _$STATESET2$PJ$lB1$rB$pDERStateSetJac0$PdummyVarStateSetJac0;
     modelica_real tmp126;
     modelica_real tmp127;
     modelica_real tmp128;
     modelica_real tmp129;
     modelica_real tmp130;
     modelica_real tmp131;
     modelica_real tmp132;
     modelica_real tmp133;
     for (int col = 0; col < 4; col++)
     {
         _S_[col] = 1.0;
         tmp126 = cos(_q2);
         tmp127 = sin(_q2);
         tmp128 = sin(_q2);
         tmp129 = cos(_q2);
         _$STATESET2$PJ$lB2$rB$pDERStateSetJac0$PdummyVarStateSetJac0 = (-2.0 * (((_x + (_xp + (_L * tmp126))) * (_x$pDERStateSetJac0$Px + (_L * ((-tmp127) * _q2$pDERStateSetJac0$Pq2)))) + ((_z + (_L * tmp128)) * (_z$pDERStateSetJac0$Pz + (_L * (tmp129 * _q2$pDERStateSetJac0$Pq2)))))); 
         tmp130 = cos(_q1);
         tmp131 = sin(_q1);
         tmp132 = sin(_q1);
         tmp133 = cos(_q1);
         _$STATESET2$PJ$lB1$rB$pDERStateSetJac0$PdummyVarStateSetJac0 = (-2.0 * (((_x + ((-_xp) - (_L * tmp130))) * (_x$pDERStateSetJac0$Px - (_L * ((-tmp131) * _q1$pDERStateSetJac0$Pq1)))) + ((_z + (_L * tmp132)) * (_z$pDERStateSetJac0$Pz + (_L * (tmp133 * _q1$pDERStateSetJac0$Pq1)))))); 
         _S_[col] = 0.0;
         _Jacobian_StateSetJac0[0+2*col] = _$STATESET2$PJ$lB2$rB$pDERStateSetJac0$PdummyVarStateSetJac0;
         _Jacobian_StateSetJac0[1+2*col] = _$STATESET2$PJ$lB1$rB$pDERStateSetJac0$PdummyVarStateSetJac0;
     }
 }
 

 bool Robot::selectStateVars()
 {
     bool doReinit = false;
     calc_Jacobian_StateSetJac1();
     if (selectDynamicStates(_Jacobian_StateSetJac1,2,4,rowSelect_$STATESET1_A,colSelect_$STATESET1_A))
     {
         for (int row = 0; row < 2; row++)
             for (int col = 0; col < 4; col++)
                 _$STATESET1_A.get(row,col) = 0;
         for (int row = 0; row < 2; row++)
         {
             int rowIndex = rowSelect_$STATESET1_A[(2-1)-row];
             int colIndex = colSelect_$STATESET1_A[(4-1)-row];
             _$STATESET1_A.get(rowIndex,colIndex) = 1;
         }
         doReinit = true;
     }
     _$STATESET1_x.get(0) = _dz*_$STATESET1_A.get(0,0)+_dq2*_$STATESET1_A.get(0,1)+_dx*_$STATESET1_A.get(0,2)+_dq1*_$STATESET1_A.get(0,3);
     _$STATESET1_x.get(1) = _dz*_$STATESET1_A.get(1,0)+_dq2*_$STATESET1_A.get(1,1)+_dx*_$STATESET1_A.get(1,2)+_dq1*_$STATESET1_A.get(1,3);
     calc_Jacobian_StateSetJac0();
     if (selectDynamicStates(_Jacobian_StateSetJac0,2,4,rowSelect_$STATESET2_A,colSelect_$STATESET2_A))
     {
         for (int row = 0; row < 2; row++)
             for (int col = 0; col < 4; col++)
                 _$STATESET2_A.get(row,col) = 0;
         for (int row = 0; row < 2; row++)
         {
             int rowIndex = rowSelect_$STATESET2_A[(2-1)-row];
             int colIndex = colSelect_$STATESET2_A[(4-1)-row];
             _$STATESET2_A.get(rowIndex,colIndex) = 1;
         }
         doReinit = true;
     }
     _$STATESET2_x.get(0) = _z*_$STATESET2_A.get(0,0)+_q1*_$STATESET2_A.get(0,1)+_x*_$STATESET2_A.get(0,2)+_q2*_$STATESET2_A.get(0,3);
     _$STATESET2_x.get(1) = _z*_$STATESET2_A.get(1,0)+_q1*_$STATESET2_A.get(1,1)+_x*_$STATESET2_A.get(1,2)+_q2*_$STATESET2_A.get(1,3);
     return doReinit;
 }
 
 double Robot::calcDelay(int index, double expr, double t, double delay)
 {
     if (delays[index] == NULL || !delays[index]->isEnabled()) return expr;
     else return delays[index]->sample(t-delay);
 }
 
 void Robot::saveDelay(int index, double expr, double t, double max_delay)
  {
      if (delays[index] == NULL)
          delays[index] = new AdevsDelayData(max_delay);
      delays[index]->insert(t,expr);
  }
 
