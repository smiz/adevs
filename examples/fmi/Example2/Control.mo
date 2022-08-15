model Control extends ControlTrajectory;
  // Joint trajectory
  Real qd1(start=0), qd2(start=0);
  // Motor x coordinate 
  parameter Real xp = 0.1;
  // Lengths 
  parameter Real L = 0.3, l = 0.7;
equation
  // Position and angle constraints (Eqn 4)
  0 = (xd-xp-L*cos(qd1))^2+(zd+L*sin(qd1))^2-l^2;
  0 = (xd+xp+L*cos(qd2))^2+(zd+L*sin(qd2))^2-l^2;
end Control;
