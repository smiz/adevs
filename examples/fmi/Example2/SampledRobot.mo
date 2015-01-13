model Robot extends BaseRobot;
  parameter Real sampleFreq = 1000.0; 
algorithm
  if initial() or sample(0,1.0/sampleFreq) then
    q1_sample := q1;
    q2_sample := q2;
	sampleNumber := sampleNumber+1;
  end if;
end Robot;

