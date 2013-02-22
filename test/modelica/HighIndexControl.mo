model Control
	// Desired trajectory
	Real xd(start=0);
	Real zd(start=0);
	// Joint trajectory
	Real qd1(start=0);
	Real qd2(start=0);
	// Output torque commands
	output Real T[2];
	// Input positions
	parameter Real q1(start=0);
	parameter Real q2(start=0);
	// Control gains
	parameter Real Kc = 600;
	parameter Real Ti = 300;
	parameter Real Td = 0.05;
	// Control error
	Real e[2];
	Real ei[2];
	Real ed[2];
	// Motor x coordinate 
	parameter Real xp = 0.1;
	// Lengths 
	parameter Real L = 0.3, l = 0.7;
	// Masses
	parameter Real m1 = 0.82, m2 = 0.14,
		m3 = 0.5, ml = 5.0;
equation
	xd = -0.35*sin(2*3.14*time);
	zd = -0.7+0.1*cos(2*3.14*time);
	// Position and angle contraints (Eqn 4)
	0 = (xd-xp-L*cos(qd1))^2+(zd+L*sin(qd1))^2-l^2;
	0 = (xd+xp+L*cos(qd2))^2+(zd+L*sin(qd2))^2-l^2;
	e[1] = qd1-q1;
	e[2] = qd2-q2;
	ei = Ti*der(e);
	ed = Td*(e-delay(e,0.01))/0.01;
	T = Kc*(e+ei+ed); 
end Control;
