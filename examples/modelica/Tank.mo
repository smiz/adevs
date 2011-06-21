model Motor
	protected Modelica.Electrical.Analog.Basic.Resistor Rm(R = 3.1); 
	protected Modelica.Electrical.Analog.Basic.Inductor Lm(L = 1E-3);
	protected Modelica.Electrical.Analog.Basic.Ground ground;
	protected Modelica.Electrical.Analog.Basic.EMF emf(k = 1E-3);
	public Modelica.Electrical.Analog.Interfaces.Pin terminal;
	public Modelica.Mechanics.Rotational.Interfaces.Flange_b shaft;
equation
	connect(terminal,Rm.p);
	connect(Rm.n,Lm.p);
	connect(Lm.n, emf.p);
	connect(ground.p, emf.n);
	connect(emf.flange,shaft);
end Motor;

model GearsAndTrack
	protected Modelica.Mechanics.Rotational.Components.IdealGearR2T r2t(
		ratio = 1.0/0.015);
	protected Modelica.Mechanics.Rotational.Components.IdealGear gear(ratio = 204);
	protected Modelica.Mechanics.Rotational.Components.Inertia Jg(J = 1.2E-6);
	protected Modelica.Mechanics.Rotational.Components.Damper Bg(d = 6.7E-7);
	protected Modelica.Mechanics.Translational.Components.Spring K(c = 1000);
	protected Modelica.Mechanics.Rotational.Components.Fixed Rfixed;
	public Modelica.Mechanics.Rotational.Interfaces.Flange_a motor_shaft;
	public Modelica.Mechanics.Translational.Interfaces.Flange_b track;
equation
	connect(motor_shaft,Jg.flange_a);
	connect(Jg.flange_b,Bg.flange_a);
	connect(Jg.flange_b,gear.flange_a);
	connect(Bg.flange_b,Rfixed.flange);
	connect(gear.flange_b,r2t.flangeR);
	connect(r2t.flangeT,K.flange_a);
	connect(K.flange_b,track);
end GearsAndTrack;

model Hull
	protected parameter Real Jt = 5E-4;
	protected parameter Real mt = 0.8;
	protected parameter Real B = 0.1;
	protected parameter Real Br = 1.0;
	protected parameter Real Bs = 14.0;
	protected parameter Real Bl = 0.7;
	protected parameter Real Sl = 0.3;
	protected Real x(start = 0);
	protected Real y(start = 0);
	protected Real v(start = 0);
	protected Real omega(start = 0);
	protected Real theta(start = 0);
	protected Real Fl, Fr;
	protected Real tau;
	protected Integer turning(start = 0);
	public Modelica.Mechanics.Translational.Interfaces.Flange_a left_track;
	public Modelica.Mechanics.Translational.Interfaces.Flange_a right_track;
equation
	der(v) = ((Fl+Fr)-(Br+turning*Bs)*v)/mt;
	tau = B*(Fl-Fr)/2;
	turning = if (tau > Sl or tau < -Sl) then 1 else 0;
	der(omega) = turning*(tau-Bl*omega)/Jt;
	der(theta) = turning*omega;
	der(x) = v*sin(theta);
	der(y) = v*cos(theta);
	Fl = left_track.f;
	Fr = right_track.f;
	der(left_track.s) = v+turning*B*omega/2;
	der(right_track.s) = v-turning*B*omega/2;
end Hull;

model Propulsion
	protected Motor motor;
	protected GearsAndTrack gear_and_track;
	public Modelica.Electrical.Analog.Interfaces.Pin terminal;
	public Modelica.Mechanics.Translational.Interfaces.Flange_a track;
equation
	connect(terminal, motor.terminal);
	connect(motor.shaft, gear_and_track.motor_shaft);
	connect(gear_and_track.track, track);
end Propulsion;

model Tank
	parameter Real vin_left = 7.2, vin_right = 7.2;
	Propulsion left_propulsion;
	Propulsion right_propulsion;
	Hull hull;
	Modelica.Electrical.Analog.Basic.Ground ground;
	Modelica.Electrical.Analog.Sources.SignalVoltage sig_left, sig_right;
equation
	sig_left.v = vin_left;
	sig_right.v = vin_right;
	connect(sig_left.n, ground.p);
	connect(sig_right.n, ground.p);
	connect(sig_left.p, left_propulsion.terminal);
	connect(sig_right.p, right_propulsion.terminal);
	connect(left_propulsion.track,hull.left_track);
	connect(right_propulsion.track,hull.right_track);
end Tank;

