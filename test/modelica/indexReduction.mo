model indexReduction
	Real x1, x2, x3, x4, dx1, dx2, dx3;
equation
  der(x1)=dx1;
  der(x2)=dx2;
  der(x3)=dx3;
  x4*x3*x2*x1^3+x2^3*x1= 0;
  (x1*x2+x3*x2)^2=0;
  x1+der(x3)+x4=0;
  2*der(dx1)+der(dx2)+der(dx3)+der(x4)=0;
end indexReduction;

