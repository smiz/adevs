#ifndef __node_h_
#define __node_h_
#include <iostream>
#include <cstdio>
#include <cassert>
#include "adevs.h"

struct token_t
{
	int value;
	token_t(int value = 0):
	value(value)
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
		}
		double ta()
		{
			return sigma;
		}
		void delta_int()
		{
			assert(token != NULL);
			t += ta();
			printf("%d sent %d @ t = %.0f\n",ID,out_token->value,t);
			out_token = token;
			sigma = DBL_MAX;
		}
		void delta_ext(double e, const adevs::Bag<PortValue>& x)
		{
			t += e;
			assert (x.size() == 1);
			token = static_cast<token_t*>((*(x.begin())).value);
			if (out_token == NULL) out_token = token;
			sigma = holdtime;
			printf("%d got %d @ t = %.0f\n",ID,token->value,t);
		}
		void delta_conf(const adevs::Bag<PortValue>& x) 
		{
			t += ta();
			assert (x.size() == 1);
			printf("%d sent %d @ t = %.0f\n",ID,token->value,t);
			out_token = token = static_cast<token_t*>((*(x.begin())).value);
			sigma = holdtime;
			printf("%d got %d @ t = %.0f\n",ID,token->value,t);
		}
		void output_func(adevs::Bag<PortValue>& y)
		{
			PortValue pv(out,out_token);
			y.insert(pv);
		}
		void gc_output(adevs::Bag<PortValue>&)
		{
		}
		~node()
		{
		}
	private:	
		int ID;
		token_t* token, *out_token;
		double holdtime, sigma, t;
};
		
const int node::in(0);
const int node::out(1);

#endif
