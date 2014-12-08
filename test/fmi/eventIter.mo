class eventIter
	Real c(start=0);
	Real floorc;
	Integer high(start=0);
equation
	der(c) = 1.0;
	floorc = c-floor(c);
	high = if floorc < 0.25 then 0 else 1;
	when floorc > 0.5 then
		reinit(c,0.0);
	end when;	
initial equation
	c = 0;
	high = 0;
end eventIter;
