model ControlTrajectory
  // Desired trajectory
  Real xd(start=0), zd(start=0);
  parameter Real pi = 3.141592653589793;
equation
  xd = -0.35*sin(pi*time/2.0);
  zd = -0.7+0.1*cos(pi*time/2.0);
end ControlTrajectory;
