// Test case for built in mathematical functions.

class builtins_events
	Real x(start = 0.5,fixed=true);
	output Real y_ceil(start = 0);
	output Real y_floor(start = 0);
	output Integer y_div(start = 0);
	output Real y_mod(start = 0);
	output Integer y_rem(start = 0);
	output Real y_int(start = 0);
equation
	der(x) = 0.1;
	y_ceil = ceil(x);
	y_floor = floor(x);
	y_div = div(sign(x),x);
	y_mod = mod(2.0,x);
	y_rem = rem(x,x);
	y_int = integer(x);
end builtins_events;
