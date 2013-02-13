model Linsys
	parameter Real [2,2] A = { { -0.5, 0.0 }, { 0.0, -1.0 } };
	parameter Real [2,2] B = { { 0.0, 0.1 }, { 0.1, 0.0 } };
	parameter Real [2,2] C = { { 1.0, 0.0 }, { 0.0, 1.0 } };
	Real [2] x;
	input Real [2] u;
	output Real [2] y;
equation
	der(x) = A*x+B*u;
	y = C*x;
initial equation
	x[1] = 1.0;
	x[2] = 2.0;
end Linsys;

model Linsys2
	Linsys [2] system;
equation
	connect(system[1].y,system[2].u);
	connect(system[2].y,system[1].u);
end Linsys2;
