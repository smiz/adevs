class Bounce
  Real x(start = 0.5);
  Real y(start = 0.5);
  parameter Real a = 1;
equation
  der(x) = if (y <= 1) then -a else a;
  der(y) = if (x >= 0) then a else -a;
end Bounce;
