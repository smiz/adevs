model CherryBomb
	Real h(start=1); // Height of the bomb
	Real v(start=0); // Velocity of the bomb
	parameter Real g = 9.8; // Gravity
	Real fuseTime(start=2.0); // Fuse duration
	input Boolean dropped(start=false); 
	output Boolean exploded(start=false);
equation
	// Accelerate downward if we have been lit
	der(v) = if (not dropped) then 0 else -g;
	der(h) = v;
	// Burn the fuse
	der(fuseTime) = if (not dropped) then 0 else -1;
	when {h <= 0} then
		reinit(v,-pre(v));
	end when;
	when {fuseTime <= 0} then
		exploded = true;
	end when;
end CherryBomb;


