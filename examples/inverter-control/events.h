#ifndef _events_h_
#define _events_h_

struct event_t
{
	#define ABC_SAMPLE 0
	#define SET_SWITCH 1
	#define PWM_DUTY_CYCLE 2
	int type;
	union
	{
		struct
		{
			double iabc[3];
			double vabc[3];
		} data;
		double pwm[3];
		struct
		{
			int channel;
			double polarity;
		} open_close;
	} value;
};

#endif
