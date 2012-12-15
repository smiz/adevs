// Test case for built in mathematical functions.

class builtins_events
	Real x(start = 1, fixed = true);
	Real y;
	Integer n(start=9);
	output Real y_ceil(start = 0);
	output Real y_floor(start = 0);
	output Real y_div1(start = 0);
	output Real y_div2(start = 0);
	output Real y_mod1(start = 0);
	output Real y_mod1_compare(start = 0);
	output Real y_mod2(start = 0);
	output Real y_mod2_compare(start = 0);
	output Real y_rem1(start = 0);
	output Real y_rem2(start = 0);
	output Integer y_int(start = 0);
equation
	der(x) = -x;
	y = 10.0*x;
	when x < 0.25 then
		reinit(x,-1.0);
	end when;
	when x > -0.05 then
		reinit(x,1.0);
	end when;
algorithm
	when abs(y) < n then
		n := n - 1;
		y_ceil := ceil(y);
		y_floor := floor(y);
		y_div1 := div(y,x);
		y_div2 := div(x,y);
		y_mod1 := mod(y,x);
		y_mod1_compare := y - floor(y/x)*x;
		y_mod2 := mod(x,y);
		y_mod2_compare := x - floor(x/y)*y;
		y_rem1 := rem(x,y);
		y_rem2 := rem(y,x);
		y_int := integer(y);
	end when;
initial algorithm
	n := 9;
end builtins_events;

