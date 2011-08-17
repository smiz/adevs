/**
 * This is a model of a power system comprising
 * a single generator and single load.
 */
model PowerSys
	Real w(start = 0);
	Real theta(start = 0,fixed=false);
	Real Pm(start = 1,fixed=false);
	Real Pe;
	Real E(start=1,fixed=false);
	Complex V;
	Complex I;
	parameter Real M = 5;
	parameter Real Tw = 1;
	parameter Real Te = 10;
	parameter Real Vref = 1;
	parameter Complex y = Complex(1.0,1.0);
	parameter Complex Xd = Complex(0.0,1.0);
equation
	der(theta) = w;
	der(w) = (Pm-Pe)/M;
	der(Pm) = -Tw*w;
	der(E) = Te*(Vref-'abs'(V));
	I = fromPolar(E,theta)/Xd;
	I = V*(1/Xd-y);
initial equation
	der(theta) = 0;
	der(E) = 0;
	der(Pm) = 0;
end PowerSys;

