model Circuit "This is the circuit illustrated in tutorials/ex5.cpp"
	import Resistor = Modelica.Electrical.Analog.Basic.Resistor;
	import Capacitor = Modelica.Electrical.Analog.Basic.Capacitor;
	import Ground = Modelica.Electrical.Analog.Basic.Ground;
	import ConstantVoltage = Modelica.Electrical.Analog.Sources.ConstantVoltage;
	import Diode = Modelica.Electrical.Analog.Ideal.IdealDiode;
	import Switch = Modelica.Electrical.Analog.Ideal.IdealOpeningSwitch;
	Resistor Rs(R=1.0);
	Resistor Rl(R=10.0);
	Capacitor C(C=1E-1);
	ConstantVoltage Vs(V=1.0);
	Diode D(Vknee=0.25,Ron=1E-11,Goff=1E-11);
	Switch S(Ron=1E-11,Goff=1E-11);
	Ground gnd;

	input Boolean switch;

equation
	connect(Vs.n,gnd.p);
	connect(Vs.p,Rs.p);
	connect(Rs.n,S.p);
	connect(S.n,C.p);
	connect(C.n,gnd.p);
	connect(C.p,D.p);
	connect(D.n,Rl.p);
	connect(Rl.n,gnd.p);
	switch = S.control;
end Circuit;


