model Control
	// Desired trajectory
	Real xd(start=0);
	Real zd(start=0);
	// Joint trajectory
	Real qd1(start=0);
	Real qd2(start=0);
	// Motor x coordinate 
	parameter Real xp = 0.1;
	// Lengths 
	parameter Real L = 0.3, l = 0.7;
	parameter Real pi = 3.141592653589793;
equation
	xd = -0.35*sin(pi*time/2.0);
	zd = -0.7+0.1*cos(pi*time/2.0);
	// Position and angle contraints (Eqn 4)
	0 = (xd-xp-L*cos(qd1))^2+(zd+L*sin(qd1))^2-l^2;
	0 = (xd+xp+L*cos(qd2))^2+(zd+L*sin(qd2))^2-l^2;
end Control;
