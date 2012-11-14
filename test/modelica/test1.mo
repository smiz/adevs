class test1
	Real x(start = 0);
	parameter Real a(start = -1);
	Boolean goUp(start = false);
	Boolean goDown(start = false);
	Boolean xAbove(start=false);
	Boolean aAbove(start=false);
equation
	der(x) = a*x;
	xAbove = if (x > 1.5) then true else false;
//	goUp = if (x <= 1.5 and a < 0) then true else false;
//	goDown = if (x >= 1.5 and a > 0) then true else false;
	when goUp then
		reinit(x,1);
	end when;
	when goDown then
		reinit(x,2);
	end when;
algorithm
	if a > 0 then
		aAbove := true;
	else
		aAbove := false;
	end if;
	goUp := not aAbove and not xAbove;
	goDown := aAbove and xAbove;
initial equation
	x = 2;
end test1;
