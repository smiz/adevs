class stairs
	Real x(start = 0), step;
	parameter Real a = 1;
equation
	der(x) = a;
	step = floor(x);
end stairs;
