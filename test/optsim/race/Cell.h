#ifndef _Cell_h_
#define _Cell_h_
#include <cstring>

#define MAX_SPEED 2.0
struct car_t
{
	int ID;
	double spd;
};

class Cell:
	public adevs::Atomic<car_t*>
{
	public:
		int getPos() const { return pos; }
		Cell(int pos, car_t* car = NULL):
			adevs::Atomic<car_t*>(),
			car(car),
			t(0.0),
			pos(pos)
		{
			assignToLP(pos);
			msg[0] = '\0';
		}
		void delta_int()
		{
			t += ta();
			sprintf(msg,"%f: car %d left %d!",t,car->ID,pos);
			delete car;
			car = NULL;
		}
		void delta_ext(double e, const adevs::Bag<car_t*>& xb)
		{
			t+= e;
			assert(xb.size() == 1);
			car_t* tmp_car = *(xb.begin());
			sprintf(msg,"%f: car %d is at %d!",t,tmp_car->ID,pos);
			if (car != NULL)
			{
				char tmp[100];
				sprintf(tmp,"\n%f: collision @ %d!",t,pos);
				strcat(msg,tmp);
				delete car;
				car = NULL;
			}
			else
			{
				car = new car_t(*(*(xb.begin())));
			}
		}
		void delta_conf(const adevs::Bag<car_t*>& xb)
		{
			int car_left_id = car->ID;
			delta_int();
			delta_ext(0.0,xb);
			char tmp[100];
			sprintf(tmp,"\n%f: car %d left %d!",t,car_left_id,pos);
			strcat(msg,tmp);
		}
		double lookahead() { return 1.0/MAX_SPEED; }
		double ta()
		{
			if (car == NULL) return DBL_MAX;
			else return 1.0/car->spd;
		}
		void output_func(adevs::Bag<car_t*>& yb)
		{
			yb.insert(new car_t(*car));
		}
		void gc_output(adevs::Bag<car_t*>& gb)
		{
			adevs::Bag<car_t*>::iterator iter = gb.begin();
			for (; iter != gb.end(); iter++)
				delete *iter;
		}
		void* save_state()
		{
			state_t* s = new state_t;
			s->car = NULL;
			if (car != NULL) s->car = new car_t(*car);
			s->t = t;
			strcpy(s->msg,msg);
			return s;
		}
		void restore_state(void* data)
		{
			state_t* s = (state_t*)data;
			if (car != NULL) delete car;
			car = NULL;
			if (s->car != NULL) car = new car_t(*(s->car));
			t = s->t;
			strcpy(msg,s->msg);
		}
		void gc_state(void* data)
		{
			delete (state_t*)data;
		}
		const char* getMsg()
		{
			return msg;
		}
		double getTime()
		{
			return t;
		}
	private:
		car_t* car;
		double t;
		const int pos;
		char msg[100];
		struct state_t
		{
			car_t* car;
			double t;
			char msg[100];
		};
};

#endif
