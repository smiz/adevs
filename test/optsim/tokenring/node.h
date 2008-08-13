#ifndef __node_h_
#define __node_h_
#include <iostream>
#include <cassert>
#include "adevs.h"

struct token_t
{
	int value;
	token_t(int value = 0):
	value(value)
	{}
	token_t(const token_t& src):
	value(src.value)
	{}
};

typedef adevs::PortValue<token_t*> PortValue;

class node: public adevs::Atomic<PortValue>
{
	public:
		static const int in;
		static const int out;

		node(int ID, int holdtime, token_t* token):
		adevs::Atomic<PortValue>(),
		ID(ID),
		token(token),
		out_token(token),
		holdtime(holdtime),
		sigma(DBL_MAX),
		t(0.0)
		{
			if (token != NULL)
			{
				sigma = holdtime;
			}
			msg[0] = '\0';
		}
		double ta()
		{
			return sigma;
		}
		void delta_int()
		{
			assert(token != NULL);
			t += ta();
			sprintf(msg,"%d sent %d @ t = %.0f\n",ID,out_token->value,t);
			// Copy the internal token if we are going to reuse it
			if (out_token == token) token = new token_t(*token);
			out_token = token;
			sigma = DBL_MAX;
		}
		void delta_ext(double e, const adevs::Bag<PortValue>& x)
		{
			t += e;
			assert (x.size() == 1);
			token = static_cast<token_t*>((*(x.begin())).value);
			// Make a copy of the input
			token = new token_t(*token);
			if (out_token == NULL) out_token = token;
			sigma = holdtime;
			sprintf(msg,"%d got %d @ t = %.0f\n",ID,token->value,t);
		}
		void delta_conf(const adevs::Bag<PortValue>& x) 
		{
			t += ta();
			assert (x.size() == 1);
			sprintf(msg,"%d sent %d @ t = %.0f\n",ID,token->value,t);
			// Make a copy of the input
			token_t* tmp = static_cast<token_t*>((*(x.begin())).value);
			out_token = token = new token_t(*tmp);
			sigma = holdtime;
			char tmp_msg[100];
			sprintf(tmp_msg,"%d got %d @ t = %.0f\n",ID,token->value,t);
			strcat(msg,tmp_msg);
		}
		void output_func(adevs::Bag<PortValue>& y)
		{
			PortValue pv(out,out_token);
			y.insert(pv);
		}
		void gc_output(adevs::Bag<PortValue>& gb)
		{
			adevs::Bag<PortValue>::iterator iter = gb.begin();
			for (; iter != gb.end(); iter++) delete (*iter).value;
		}
		~node()
		{
		}
		static const char* getMessage(void* state)
		{
			state_t* s = (state_t*)state;
			return s->msg;
		}
		static double getTime(void* state)
		{
			state_t* s = (state_t*)state;
			return s->t;
		}
	private:	
		const int ID;
		token_t* token, *out_token;
		const double holdtime;
		double sigma, t;
		char msg[100];

		struct state_t
		{
			token_t *token, *out_token;
			double sigma, t;
			char msg[100];
		};
		void* save_state()
		{
			state_t* s = new state_t;
			strcpy(s->msg,msg);
			s->sigma = sigma;
			s->t = t;
			s->token = s->out_token = NULL;
			if (token != NULL) s->token = new token_t(*token);
			if (out_token == token) s->out_token = s->token;
			else if (out_token != NULL) s->out_token = new token_t(*out_token);
			return s;
		}
		void restore_state(void* data)
		{
			state_t* s = (state_t*)data;
			strcpy(msg,s->msg);
			sigma = s->sigma;
			t = s->t;
			if (token != NULL) delete token;
			if (out_token != NULL && out_token != token) delete out_token;
			token = out_token = NULL;
			if (s->token != NULL) token = new token_t(*(s->token));
			if (s->out_token == s->token) out_token = token;
			else if (s->out_token != NULL) out_token = new token_t(*(s->out_token));
		}
		void gc_state(void* data)
		{
			delete (state_t*)data;
		}
};
		
const int node::in(0);
const int node::out(1);

#endif
