model BaseRobot extends ControlTrajectory;
  // Position 
  Real x(start=0.0), z(start=-0.6);
  // Joint angles 
  Real q1(start=0.0838403), q2(start=0.0818715);
  // Derivatives of positions and angles
  Real dx, dz;
  Real dq1, dq2;
  // Motor x coordinate 
  parameter Real xp = 0.1;
  // Lengths 
  parameter Real L = 0.3, l = 0.7;
  // Masses
  parameter Real m1 = 0.82, m2 = 0.14,
    m3 = 0.5, ml = 5.0;
  // Rotational inertias
  parameter Real Jmot = 0.37E-4,
    Jred = 9.09E-4, I = 0.018895002;
  // Friction coefficients
  parameter Real Fs = 3.0, Fv = 0.5;
  // Gear ratio
  parameter Real nu = 5.0;
  // Gravity
  parameter Real g = 9.81;
  // Torques
  Real Tred[2], Tfric[2],
    Tarm[2], Tfarm[2], Ttplate[2];
  // Control inputs
  parameter Real T[2] = {0,0};
  // Mass and inertia matrices
  Real JxT[2,2], JqInv[2,2], M[2];
  // How many samples have been generated 
  Integer sampleNumber(start=0,fixed=true);
  // Sample values
  output Real q1_sample, q2_sample;
  // Control error
  Real error;
equation
  error = abs(x-xd)+abs(z-zd);
  // Position and angle contraints 
  0 = (x-xp-L*cos(q1))^2+(z+L*sin(q1))^2-l^2;
  0 = (x+xp+L*cos(q2))^2+(z+L*sin(q2))^2-l^2;
  // State variable derivatives
  der(q1) = dq1;
  der(q2) = dq2;
  der(x) = dx;
  der(z) = dz;
  // Torques 
  T = Tred+Tfric+Tarm+Tfarm+Ttplate;
  Tred[1] = (nu^2)*(Jmot+Jred)*der(dq1);
  Tred[2] = (nu^2)*(Jmot+Jred)*der(dq2);
  Tfric[1] = sign(dq1)*Fs+Fv*dq1;
  Tfric[2] = sign(dq2)*Fs+Fv*dq2;
  Tarm[1] = I*der(dq1)-m1*g*cos(q1);
  Tarm[2] = I*der(dq2)-m1*g*cos(q2);
  Tfarm[1] = 0.5*m2*L*(L*der(dq1)-g*cos(q1));
  Tfarm[2] = 0.5*m2*L*(L*der(dq2)-g*cos(q2));
  // Jacobians 
  JxT[1,1] = 2*(x-xp-L*cos(q1));
  JxT[1,2] = 2*(x+xp+L*cos(q2));
  JxT[2,1] = 2*(z+L*sin(q1));
  JxT[2,2] = 2*(z+L*sin(q2));
  JqInv[1,1] = 1/(2*(x-xp-L*cos(q1))*L*sin(q1)+
      2*(z+L*sin(q1))*L*cos(q1));
  JqInv[1,2] = 0;
  JqInv[2,1] = 0;
  JqInv[2,2] = 1/(2*(x+xp+L*cos(q2))*(-L*sin(q2))+
      2*(z+L*sin(q2))*L*cos(q2));
  M = (JxT*JqInv)*Ttplate;
  0 = (m2+m3+ml)*der(dx)+M[1];
  0 = (m2+m3+ml)*(der(dz)-g)+M[2];
initial equation
  x = 0;
  z = -0.6;
end BaseRobot;
