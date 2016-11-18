class eventIter
	Real c(start=0);
	Real floorc;
	output Integer high;
equation
	der(c) = 1.0;
	floorc = c-floor(c);
	high = if c < 0.25 then 0 else 1;
	when c > 0.5 then
		reinit(c,0.0);
	end when;	
end eventIter;
