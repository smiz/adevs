model lk
  parameter Real a=0.1;
  parameter Real b=0.002;
  parameter Real c=0.2;
  parameter Real d=0.0025;
  Real x(start=80.0,fixed=true),y(start=20.0,fixed=true);
equation
  der(x)=a*x-b*x*y;
  der(y)=d*x*y-c*y;
end lk;

