class test1
	Real x(start = 0);
	parameter Real a(start = -1);
	Boolean goUp(start = false);
	Boolean goDown(start = false);
equation
	der(x) = a*x;
	goUp = if (x <= 1.5 and a < 0) then true else false;
	goDown = if (x >= 1.5 and a > 0) then true else false;
	when goUp then
		reinit(x,1);
	end when;
	when goDown then
		reinit(x,2);
	end when;
initial equation
	x = 2;
end test1;
