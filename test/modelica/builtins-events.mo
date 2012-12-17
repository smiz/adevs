// Test case for built in mathematical functions.

class builtins_events
	Real x(start = 1, fixed = true);
	Real y;
	output Real y_ceil(start = 0);
	output Real y_floor(start = 0);
	output Integer y_div(start = 0);
	output Real y_div_expr(start=0);
	output Real y_mod1(start = 0);
	output Real y_mod1_compare(start = 0);
	output Real y_mod2(start = 0);
	output Real y_mod2_compare(start = 0);
	output Integer y_rem(start = 0);
	output Integer y_int(start = 0);
	output Integer y_mod_int(start = 0);
equation
	der(x) = -x;
	y = 10.0*x;
	y_ceil = ceil(y);
	y_floor = floor(y);
	y_div_expr = 2.0*sin(10.0*time)/x;
	y_div = div(2.0*sin(10.0*time),x);
	y_mod1 = mod(y,x);
	y_mod_int = mod(y_int+1,y_int);
	y_mod1_compare = y - floor(y/x)*x;
	y_mod2 = mod(x,y);
	y_mod2_compare = x - floor(x/y)*y;
	y_rem = rem(x,x);
	y_int = integer(y);
	when x < 0.5 then
		reinit(x,-1.0);
	end when;
	when x > -0.5 then
		reinit(x,1.0);
	end when;
end builtins_events;
