/**
 * This model is used to test the
 * initialization function of the adevs
 * runtime system.
 */
class Population
  Real ps(start = 10,fixed=false);
  Real pa(start = 100,fixed=false);
  parameter Real b = 0.02;
  parameter Real c1= 1E-3;
  parameter Real c2 = 0.9;
  parameter Real c3 = 1E-4;
  parameter Real c4 = 1.1;
  parameter Real c5 = 1E-5;
equation
  der(ps) = b*c1*c2*ps*pa-c2*ps-c3*ps^2;
  der(pa) = c4*pa-c5*pa^2-c1*ps*pa;
initial equation
  der(ps) = b*c1*c2-c2-c3;
  der(pa) = c4-c5-c1;
end Population;
