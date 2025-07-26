model Robot extends BaseRobot;
	parameter Real Delta = 0.005;
equation
	q1_sample = Delta*floor(q1/Delta);
	q2_sample = Delta*floor(q2/Delta);
end Robot;

