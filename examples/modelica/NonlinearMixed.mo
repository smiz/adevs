model NonlinearMixed
  Real x(start=0);
  Real y(start=0);
  Real z(start=0);
  discrete Boolean a(start=false);
  Real xx(start=0);
equation
  a = x < -0.5 or y < -0.5 or z < -0.5;
  (x+y)^2 = if a then time else 0;
  (y+z)^2 = if a then 0 else time;
  (z+x)^2 = 0;
  der(xx) = if time<0.5 then 1 else 0;
end NonlinearMixed;

