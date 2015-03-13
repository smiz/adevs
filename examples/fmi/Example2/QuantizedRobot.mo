model Robot extends BaseRobot;
	parameter Real Delta = 0.005;
equation
	q1_sample = floor(q1/Delta);
	q2_sample = floor(q2/Delta);
end Robot;

