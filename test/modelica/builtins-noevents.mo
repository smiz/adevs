// Test case for built in mathematical functions.

class builtins_noevents
	Real x(start = 0.5,fixed=true);
	output Real y_sqrt(start = 0);
	output Real y_cos(start = 0);
	output Real y_sin(start = 0);
	output Real y_tan(start = 0);
	output Real y_acos(start = 0);
	output Real y_asin(start = 0);
	output Real y_atan(start = 0);
	output Real y_atan2(start = 0);
	output Real y_cosh(start = 0);
	output Real y_sinh(start = 0);
	output Real y_tanh(start = 0);
	output Real y_abs(start = 0);
	output Real y_log(start = 0);
	output Real y_log10(start = 0);
	output Real y_exp(start = 0);
	output Integer y_sign(start = 0);
equation
	der(x) = 0.1;
	y_sqrt = sqrt(x);
	y_cos = cos(x);
	y_sin = sin(x);
	y_tan = tan(x);
	y_acos = acos(x);
	y_asin = asin(x);
	y_atan = atan(x);
	y_cosh = cosh(x);
	y_sinh = sinh(x);
	y_tanh = tanh(x);
	y_atan2 = atan2(1.0,x);
	y_abs = abs(x);
	y_log = log(x);
	y_log10 = log10(x);
	y_exp = exp(x);
	y_sign = sign(x);
end builtins_noevents;
