connector Pin
	flow Real i;
	Real v(start=0);
end Pin;

class Ground
	Pin T;
equation
	T.v = 0;
end Ground;

class Resistor
	parameter Real R = 1;
	Pin T1;
	Pin T2;
equation
	T1.v-T2.v = R*T1.i;
	T1.i+T2.i = 0;
end Resistor;

class VoltageSource
	parameter Real Vref =1;
	Pin T;
equation
	der(T.v) = Vref-T.v;
initial equation
	der(T.v) = 0;
end VoltageSource;

class Circuit
	VoltageSource Vsrc;
	Resistor R1(R=1);
	Resistor R2(R=1);
	Resistor R3(R=1);
	Resistor R4(R=1);
	Resistor Rbridge(R=1);
	Ground Gnd;
equation
	connect(Vsrc.T,R1.T1);
	connect(Vsrc.T,R2.T1);
	connect(R1.T2,Rbridge.T1);
	connect(R2.T2,Rbridge.T2);
	connect(R1.T2,R3.T1);
	connect(R2.T2,R4.T1);
	connect(R3.T2,Gnd.T);
	connect(R4.T2,Gnd.T);
end Circuit;
