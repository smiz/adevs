model EventTests
 parameter Real p = 1;
 parameter Real x1 = p;
 parameter Real y1 = -p;
 Real x(start=0);
 Real y(start=0);
 Real a(start=p);
 Integer count(start=0);
 Boolean v1,v2,v3,v4;
 Boolean w1,w2,w3,w4;
equation
 der(x) = a;
 der(y) = -a; 
 v1 = if x>x1 then true else false;
 v2 = if x>=x1 then true else false;
 v3 = if x<x1 then true else false;
 v4 = if x<=x1 then true else false;
 w1 = if x>y then true else false;
 w2 = if x>=y then true else false;
 w3 = if x<y then true else false;
 w4 = if x<=y then true else false;
 when sample(1,1) then 
  a = pre(a)*(-1);
 end when;
 when sample(0,0.5) then
   count = pre(count) + 1;
 end when;
end EventTests;

