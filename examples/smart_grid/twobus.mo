class GeneratorWithSensor extends Thyme.Generator;
	output Integer n(start=0);
	public parameter Real freqInterval(start=0.002);
	public parameter Real nomFreq(start=60.0);
algorithm
	when nomFreq*w > (n+1)*freqInterval then
		n := n+1;
	elsewhen nomFreq*w < (n-1)*freqInterval then
		n := n-1;
	end when;
end GeneratorWithSensor;

class twobus 
	GeneratorWithSensor Genr(Pg0=2,Qg0=0,V0=2,Theta0=0,Xd=1,Xq=0);
	Thyme.ConstImpLoad Load(R=1,X=0,V0=1,Theta0=0,P0=1,Q0=0);
	Thyme.Pilink L12(R=1);
	Thyme.Bus Bus1;
	Thyme.Bus Bus2;
equation
	connect(Genr.T,Bus1.T);
	connect(L12.T1,Bus1.T);
	connect(L12.T2,Bus2.T);
	connect(Load.T,Bus2.T);
end twobus;
