#ifndef __node_h_
#define __node_h_
#include <iostream>
#include <cassert>
#include <cstring>
#include <cstdio>
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

		node(int ID, int holdtime, token_t* token, bool stutter = false):
		adevs::Atomic<PortValue>(),
		ID(ID),
		token(token),
		out_token(token),
		holdtime(holdtime),
		sigma(DBL_MAX),
		t(0.0),
		stutter(stutter)
		{
			setProc(ID);
			assert(holdtime > 0.0);
			if (token != NULL)
			{
				sigma = holdtime;
			}
			msg[0] = '\0';
		}
		double lookahead() { return holdtime; }
		double ta()
		{
			return sigma;
		}
		void delta_int()
		{
			assert(token != NULL);
			t += ta();
			if (stutter && sigma > 0.0)
			{
				sigma = 0.0;
				return;
			}
			sprintf(msg,"%d sent %d @ t = %.0f\n",ID,out_token->value,t);
			// Copy the internal token if we are going to reuse it
			if (out_token == token) token = new token_t(*token);
			// Delete the expelled token
			if (out_token != NULL) delete out_token;
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
			// Delete the expelled token
			if (out_token != NULL) delete out_token;
			out_token = token = new token_t(*tmp);
			sigma = holdtime;
			char tmp_msg[100];
			sprintf(tmp_msg,"%d got %d @ t = %.0f\n",ID,token->value,t);
			strcat(msg,tmp_msg);
		}
		void output_func(adevs::Bag<PortValue>& y)
		{
			if (stutter && sigma > 0.0) return;
			PortValue pv(out,new token_t(*out_token));
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
		const char* getMessage()
		{
			return msg;
		}
		double getTime()
		{
			return t;
		}
	private:	
		const int ID;
		token_t* token, *out_token;
		const double holdtime;
		double sigma, t;
		char msg[100];
		const bool stutter;
};
		
const int node::in(0);
const int node::out(1);

#endif
