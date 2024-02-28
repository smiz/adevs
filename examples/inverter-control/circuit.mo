

/// A switch controlled voltage
model SwitchedVoltage
	import SI = Modelica.Units.SI;
	import OnePort = Modelica.Electrical.Analog.Interfaces.OnePort;
	extends OnePort;
	/// Voltage value will take sign of switch
	parameter SI.Voltage V;
	/// Switch is 1, 0, or -1
	input Integer switch(start=0);
equation
	v = switch*V;
end SwitchedVoltage;

/**
  * Model the analog electrical circuit. This is
  * a single phase of a three phase circuit.
  * 
  *  Inverter - Switch - Linv +  Ls + <- Is + Igrid
  *                           |     |     |
  *                           C     Rs    |
  *                           |     |     |
  *                              Ground 
  * Produces current and voltage measurements
  * at a fixed sample rate. Circuit schematic.
  */

model CircuitPhase
	import Resistor = Modelica.Electrical.Analog.Basic.Resistor;
	import Capacitor = Modelica.Electrical.Analog.Basic.Capacitor;
	import Inductor = Modelica.Electrical.Analog.Basic.Inductor;
	import CurrentSource = Modelica.Electrical.Analog.Sources.SineCurrent;
	import Ground = Modelica.Electrical.Analog.Basic.Ground;
	import SI = Modelica.Units.SI;
	/// Capacitor
	Capacitor C(C=0.00114047);
	/// Inductance of the line from switch
	Inductor Linv(L=0.000279834);
	/// Inductance of the power line
	Inductor Ls(L=100E-6);
	/// Load impedance
	Resistor Rs(R=1);
	/// Circuit ground
	Ground gnd;
	/// Frequency of the power system voltage
	parameter SI.Frequency Fs = 60;
	/// Phase angle
	parameter SI.Angle PhaseAngle;
	/// Harmonic injected into the line
	CurrentSource Is(I=0.1*Vinv,f=5*Fs,phase=PhaseAngle);
	// Current from the power system
	CurrentSource Igrid(I=Vinv,f=Fs,phase=PhaseAngle);
	/// DC voltage of the inverter source
	parameter SI.Voltage Vinv = 480;
	/// Voltage switch input
	input Integer inverter_switch(start=0);
	/// Voltage switch state
	SwitchedVoltage inverter(V=Vinv);	
	/// Measured current through the resistor
	output Real i_load;
equation
	inverter_switch = inverter.switch;
	connect(inverter.n,gnd.p);
	connect(inverter.p,Linv.p);
	connect(Linv.n,C.p);
	connect(C.n,gnd.p);
	connect(Ls.p,C.p);
	connect(Ls.n,Rs.p);
	connect(Rs.n,gnd.p);
	connect(Is.p,Rs.p);
	connect(Is.n,gnd.p);
	connect(Igrid.p,Rs.p);
	connect(Igrid.n,gnd.p);
	i_load = Rs.i;
initial equation
	C.v = 0;
	Linv.i = 0;
	Ls.i = 0;
end CircuitPhase;

/**
  * The complete three phase circuit. Each
  * phase is identical except for its phase
  * angle.
  */
model ThreePhaseCircuit
	import pi = Modelica.Constants.pi; 
	CircuitPhase a(PhaseAngle=0);
	CircuitPhase b(PhaseAngle=120*pi/180);
	CircuitPhase c(PhaseAngle=-120*pi/180);
	output Real i_load[3];
	input Integer inverter_switch[3](each start = 0);
equation
	i_load[1] = a.i_load;
	i_load[2] = b.i_load;
	i_load[3] = c.i_load;
	inverter_switch[1] = a.inverter_switch;
	inverter_switch[2] = b.inverter_switch;
	inverter_switch[3] = c.inverter_switch;
end ThreePhaseCircuit;

/// abc to dq tranformation
function ABC_to_DQ
  input Real abc[3], phase_angle[3], angle;
  output Real dq[3];
algorithm
	dq[1] := (2.0/3.0)*
		(cos(angle)*abc[1]+cos(angle+phase_angle[2])*abc[2]+cos(angle+phase_angle[3])*abc[3]);
	dq[2] := (2.0/3.0)*
		(-sin(angle)*abc[1]-sin(angle+phase_angle[2])*abc[2]-sin(angle+phase_angle[3])*abc[3]);
	dq[3] := (2.0/3.0)*
		((1.0/2.0)*abc[1]+(1.0/2.0)*abc[2]+(1.0/2.0)*abc[3]);
end ABC_to_DQ;

/// dq to abc tranformation
function DQ_to_ABC
  input Real dq[3], phase_angle[3], angle;
  output Real abc[3];
algorithm
	abc[1] := cos(angle+phase_angle[1])*dq[1]-sin(angle+phase_angle[1])*dq[2]+dq[3];
	abc[2] := cos(angle+phase_angle[2])*dq[1]-sin(angle+phase_angle[2])*dq[2]+dq[3];
	abc[3] := cos(angle+phase_angle[3])*dq[1]-sin(angle+phase_angle[3])*dq[2]+dq[3];
end DQ_to_ABC;

/**
  * This is the harmonic compensation control.
  */
model HarmonicCompensator
	import Filter = Modelica.Blocks.Continuous.Filter;
	import pi = Modelica.Constants.pi; 
	// Control gains
	parameter Real G, H;
	// a, b, c phase current inputs
	input Real iabc[3](each start=0);
	// dq transformed abc signal
	Real dq[3];
	// Controler output in dq frame
	Real vdq[3];
	// Control outputs for the inverter.
	// These are duty cycles in 0 to 1.
	output Real vabc[3];
	// Maximum range of the control signal
	// before the duty cycle saturates
	parameter Real Vmax = 480;
	// Harmonic to compensate. Default
	// is the fifth to match what is
	// in the circuit model.
	parameter Real f = 300;
	// PI controller
	import PI = Modelica.Blocks.Continuous.PI;
	PI controller[3](each final k=G, each final T=H);
	// Phase angles of each line
	parameter Real PhaseAngle[3] = { 0, 120*pi/180, -120*pi/180 };
	// Filter with pass band centered on harmonic of interest
	Filter filter[3](
		each final filterType=Modelica.Blocks.Types.FilterType.BandPass,
		each final order=2,
		each final f_cut=1.1*f,
		each final f_min=0.9*f,
		each final gain=1);
	// Filtered signals
	Real filter_output[3];
equation
	filter[1].u = iabc[1];
	filter[2].u = iabc[2];
	filter[3].u = iabc[3];
	filter_output[1] = filter[1].y;
	filter_output[2] = filter[2].y;
	filter_output[3] = filter[3].y;
	dq = ABC_to_DQ(filter_output,PhaseAngle,2*pi*f*time);
	dq[1] = controller[1].u;
	dq[2] = controller[2].u;
	dq[3] = controller[3].u;
	controller[1].y = vdq[1];
	controller[2].y = vdq[2];
	controller[3].y = vdq[3];
	vabc = DQ_to_ABC(vdq,PhaseAngle,2*pi*f*time)/Vmax;
initial equation
	controller[1].x = 0;
	controller[2].x = 0;
	controller[3].x = 0;
end HarmonicCompensator;

/**
  * A continuous in time form of the inverter switch.
  */
model Inverter
	// Duty cycle describing input
	input Real duty_cycle(start=0);
	// Ouput is the switch position
	output Integer switch(start=0);
	// Frequency of the switching element
	parameter Real f;
	// Saw wave
	Real s(start=0,fixed=true);
equation
	der(s) = f;
	when s > 1 then
		reinit(s,0);
	end when;
algorithm
	switch := if (s < abs(duty_cycle)) then integer(duty_cycle/abs(duty_cycle)) else 0;
end Inverter;

// Circuit with its control system
model TestCircuit
	ThreePhaseCircuit circuit;
	HarmonicCompensator control(G=0.562996,H=0.100675);
	Inverter inverter[3](each f = 15000);
equation
	circuit.i_load = control.iabc;
	inverter[1].switch = circuit.inverter_switch[1];
	inverter[2].switch = circuit.inverter_switch[2];
	inverter[3].switch = circuit.inverter_switch[3];
	control.vabc[1] = inverter[1].duty_cycle;
	control.vabc[2] = inverter[2].duty_cycle;
	control.vabc[3] = inverter[3].duty_cycle;
end TestCircuit;
