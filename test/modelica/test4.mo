connector Pin
	flow Real i;
	Real v;
end Pin;

partial model OnePin
	Pin T;
end OnePin;

partial model TwoPin
	Pin T1;
	Pin T2;
end TwoPin;

class Resistor extends TwoPin;
	parameter Real R = 1;
equation
	T1.v-T2.v = T1.i*R;
	T1.i + T2.i = 0;
end Resistor;

class Ground extends OnePin;
equation
	T.v = 0;
end Ground;

class VoltageSource extends OnePin;
	parameter Real Vref = 0;
equation
	der(T.v) = 
		if Vref > T.v then (abs(Vref-T.v))^(1.0/3.0)
		else -(abs(Vref-T.v))^(1.0/3.0);
initial equation
	der(T.v) = 0;
end VoltageSource;

class Circuit
	VoltageSource V;
	Resistor R;
	Ground Gnd;
equation
	connect(V.T,R.T1);
	connect(R.T2,Gnd.T);
end Circuit;
