// Model of the inverted pendulum from "Building software for simulation"
class Robot
  Real x(start = 0);
  Real v(start = 0);
  Real w(start = 0);
  Real theta(start = 0.001);
  parameter Real mc = 1;
  parameter Real ma = 1;
  parameter Real Da = 1E-4;
  parameter Real Dc = 1E-4;
  parameter Real L = 0.5;
  parameter Real g = 9.8;
  parameter Real Fcontrol = 0;
equation
  der(x) = v;
  der(theta) = w;
  (mc+ma)*der(v)-L*ma*cos(theta)/2*der(w) = Fcontrol + 10*exp(-20*time) 
		- ma*L*w^2*sin(theta)/2-Dc*v;
  (-L*ma*cos(theta)/2)*der(v)+(L^2*ma/4)*der(w) = ma*L*g*sin(theta)/2-Da*w;
end Robot;
