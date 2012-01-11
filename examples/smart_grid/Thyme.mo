/**
 * This is a modelica implementation of much of the Thyme simulation
 * library. It may be compiled with OpenModelica and integrated
 * directly with discrete event simulations using Adevs. Much of
 * this is copied from the ObjectStab library, though in all cases
 * the models have been greatly simplified and adjusted for the
 * quirks of the OpenModelica compiler and runtime system.
 */
within;
package Thyme

/**
 * The electrical connectors are modelled by the voltages and 
 * currents in phasor representation:
 * I = ia + j ib 
 * V = va + j vb
 */
connector Pin "Electrical connector"
	Real va(start=1);
	Real vb;
	flow Real ia;
	flow Real ib;
end Pin;

/*
 * Shell for model with one electrical connector.
 */
partial model OnePin
	Thyme.Pin T;
end OnePin;

/*
 * Shell for model with two electrical connectors.
 */
partial model TwoPin
	Thyme.Pin T1;
	Thyme.Pin T2;
end TwoPin;

/**
 * Utility functions for handling phasors.
 */
function Pinj
	input Real Tia;
	input Real Tib;
	input Real Tva;
	input Real Tvb;
	output Real P;
algorithm
	P:= Tva*Tia + Tvb*Tib;
end Pinj;

function Qinj
	input Real Tia;
	input Real Tib;
	input Real Tva;
	input Real Tvb;
	output Real Q;
algorithm
	Q:= -Tva*Tib + Tvb*Tia;
end Qinj;

function Vmag
	input Real Tva;
	input Real Tvb;
	output Real Vmag;
algorithm
	Vmag:=sqrt(Tva*Tva + Tvb*Tvb);
end Vmag;

function Vang
	input Real Tva;
	input Real Tvb;
	output Real Vang;
algorithm
	Vang:=Modelica.Math.atan2(Tvb,Tva);
end Vang;

function Imag
	input Real Tia;
	input Real Tib;
	output Real Imag;
algorithm
	Imag:=sqrt(Tia*Tia + Tib*Tib);
end Imag;

/**
 * Utility functions for complex numbers.
 */
function ReComplexDiv
	input Real a;
	input Real b;
	input Real c;
	input Real d;
	output Real e;
algorithm
	e:=(a*c+b*d)/(c*c+d*d);
end ReComplexDiv;

function ImComplexDiv
	input Real a;
	input Real b;
	input Real c;
	input Real d;
	output Real e;
algorithm
	e:=(b*c-a*d)/(c*c+d*d);
end ImComplexDiv;

function ReComplexMult
	input Real a;
	input Real b;
	input Real c;
	input Real d;
	output Real e;
algorithm
	e:=(a*c-b*d);
end ReComplexMult;

function ImComplexMult
	input Real a;
	input Real b;
	input Real c;
	input Real d;
	output Real e;
algorithm
	e:=(a*d+b*c);
end ImComplexMult;

/**
 * A complex resistor for general use.
 */
class Impedance extends TwoPin;
	parameter Real R(start=1);
	parameter Real X(start=0);
equation
	[T1.va - T2.va; T1.vb - T2.vb] = [R, -X; X, R]*[T1.ia; T1.ib];
	[T1.ia; T1.ib] + [T2.ia; T2.ib] = [0; 0];
end Impedance;

/**
 * Ground.
 */
class Ground extends OnePin;
equation
	T.va = 0;
	T.vb = 0;
end Ground;

/**
 * This is a shunt at a bus.
 */
class Shunt extends OnePin;
	parameter Real G(start=0);
	parameter Real B(start=0);
equation
	T.va = Thyme.ReComplexDiv(T.ia,T.ib,G,B);
	T.vb = Thyme.ImComplexDiv(T.ia,T.ib,G,B);
end Shunt;

/**
 * This is a convenient placeholder for hanging other devices that
 * are attached to a bus.
 */
class Bus extends OnePin;
equation
	T.ia = 0;
	T.ib = 0;
end Bus;

/**
 * These functions are for initializing the generators
 * from load flow data.
 */
function CalcInitEf
	input Real Pg0;
	input Real Qg0;
	input Real V0;
	input Real Theta0;
	input Real Xd;
	input Real Xq;
	output Real Ef;
	protected Real Vd;
	protected Real Vq;
	protected Real Id;
	protected Real Iq;
algorithm
	Vd := V0*cos(Theta0);
	Vq := V0*sin(Theta0);
	Id := Thyme.ReComplexDiv(Pg0,Qg0,Vd,Vq);
	Iq := -Thyme.ImComplexDiv(Pg0,Qg0,Vd,Vq);
	Vd := Thyme.ReComplexMult(Id,Iq,Xd,Xq)+Vd;
	Vq := Thyme.ImComplexMult(Id,Iq,Xd,Xq)+Vq;
	Ef := Thyme.Vmag(Vd,Vq);
end CalcInitEf;

function CalcInitTheta
	input Real Pg0;
	input Real Qg0;
	input Real V0;
	input Real Theta0;
	input Real Xd;
	input Real Xq;
	output Real theta;
	protected Real Vd;
	protected Real Vq;
	protected Real Id;
	protected Real Iq;
algorithm
	Vd := V0*cos(Theta0);
	Vq := V0*sin(Theta0);
	Id := Thyme.ReComplexDiv(Pg0,Qg0,Vd,Vq);
	Iq := -Thyme.ImComplexDiv(Pg0,Qg0,Vd,Vq);
	Vd := Thyme.ReComplexMult(Id,Iq,Xd,Xq)+Vd;
	Vq := Thyme.ImComplexMult(Id,Iq,Xd,Xq)+Vq;
	theta := Thyme.Vang(Vd,Vq);
end CalcInitTheta;

/**
 * This is the generator from the THYME package. It has very simple
 * models of the speed and voltage controller.
 */
class Generator extends OnePin;
	// Starting values
	parameter Real Pg0(start=1);
	parameter Real Qg0(start=0);
	parameter Real V0(start=1);
	parameter Real Theta0(start=0);
	// Machine parameters
	parameter Real H(start=3);
	parameter Real Te(start=10);
	parameter Real Tm(start=20);
	parameter Real D(start=1);
	parameter Real Xd(start=0);
	parameter Real Xq(start=0.1);
	protected parameter Real Vref(start=V0);
	// State variables
	protected Real w(start=0);
	protected Real theta(start=Thyme.CalcInitTheta(Pg0,Qg0,V0,Theta0,Xd,Xq));
	protected Real Ef(start=Thyme.CalcInitEf(Pg0,Qg0,V0,Theta0,Xd,Xq));
	protected Real Pm(start=Pg0);
	// Algebraic variables
	protected Real V;
	protected Real Pe;
	protected Real Id;
	protected Real Iq;
	protected Real Vd;
	protected Real Vq;
equation
	// Swing equation
	der(w) = (Pm-Pe-D*w)/(2*H);
	der(theta) = w;
	// Speed governor
	der(Pm) = -Tm*w;
	// Voltage regulator
	der(Ef) = Te*(Vref-V);
	// Electrical variables in frame of machine
	V = Thyme.Vmag(Vd,Vq);
	Pe = -Thyme.Pinj(Id,Iq,Vd,Vq);
	Id = Thyme.ReComplexDiv(Ef-Vd,-Vq,Xd,Xq);
	Iq = Thyme.ImComplexDiv(Ef-Vd,-Vq,Xd,Xq);
	// Change frames from network to machine
	[T.ia ; T.ib] = [cos(theta), -sin(theta) ; sin(theta), cos(theta)] * [Id ; Iq];
	[Vd ; Vq] = [cos(-theta), -sin(-theta) ; sin(-theta), cos(-theta)] * [T.va ; T.vb];
initial equation
	w = 0;
	Pe = Pm;
	Pe = Pg0;
	V = V0;
	Vref = V;
initial algorithm
	if Ef < 0 then
		Ef := -Ef;
	end if;
end Generator;

/**
 * This is a transmission line model.
 */
class Pilink extends TwoPin;
	parameter Real R(start=1) "Series resistance";
	parameter Real X(start=0) "Series reactance";
	parameter Real B(start=0) "Series susceptance";
	parameter Real G(start=0) "Series conductance";
equation
	[T1.ia; T1.ib] =
		[G, -B; B, G]/2*[T1.va; T1.vb] + [R, X; -X, R]/(R^2 +
			X^2)*[T1.va - T2.va; T1.vb - T2.vb];
	[T2.ia; T2.ib] =
		[G, -B; B, G]/2*[T2.va; T2.vb] - [R, X; -X, R]/(R^2 +
			X^2)*[T1.va - T2.va; T1.vb - T2.vb];
end Pilink;

/**
 * An ideal transformer. T1 is the primary side.
 */
class Transformer extends TwoPin;
	Real na(start=1);
	Real nb(start=0);
equation
	T2.va = Thyme.ReComplexDiv(T1.va,T1.vb,na,nb);
	T2.vb = Thyme.ImComplexDiv(T1.va,T1.vb,na,nb);
	T1.ia = Thyme.ReComplexDiv(T2.ia,T2.ib,na,-nb);
	T1.ib = Thyme.ImComplexDiv(T2.ia,T2.ib,na,-nb);
end Transformer;

/**
 * A voltage source.
 */
class VoltageSource extends OnePin;
equation
	T.va = 1;
	T.vb = 0;
end VoltageSource;

/**
 * A constant impedance load.
 */
class ConstImpLoad extends OnePin;
	parameter Real R(start=1);
	parameter Real X(start=0);
	parameter Real P0(start=1);
	parameter Real Q0(start=0);
	parameter Real V0(start=1);
	parameter Real Theta0(start =0);
	protected Thyme.Impedance imp(R=R,X=X);
	protected Thyme.Ground gnd;
equation
	connect(gnd.T,imp.T2);
	connect(T,imp.T1);
initial equation
	V0 = Thyme.Vmag(T.va,T.vb);
	Theta0 = Thyme.Vang(T.va,T.vb);
	P0 = Thyme.Pinj(T.ia,T.ib,T.va,T.vb);
	Q0 = Thyme.Qinj(T.ia,T.ib,T.va,T.vb);
end ConstImpLoad;

end Thyme;
