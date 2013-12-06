#ifndef _io_h_
#define _io_h_
#include "adevs.h"

class SimObject
{
	public:
		SimObject(){}
		virtual ~SimObject(){}
		virtual SimObject* clone() const { return new SimObject(); }
};

typedef adevs::PortValue<SimObject*> IO_Type;

class AtomicModel:
	public adevs::Atomic<IO_Type>
{
	public:
		AtomicModel():adevs::Atomic<IO_Type>(){}
		void gc_output(adevs::Bag<IO_Type>& g)
		{
			adevs::Bag<IO_Type>::iterator iter = g.begin();
			for (; iter != g.end(); iter++)
			{
				if ((*iter).value != NULL)
					delete (*iter).value;
			}
		}
};

class CommandSig:
	public SimObject
{
	public:
		CommandSig(double T1, double T2):
			SimObject(),T1(T1),T2(T2)
		{
		}
		double getT1() const { return T1; }
		double getT2() const { return T2; }
		SimObject* clone() const
		{
			return new CommandSig(T1,T2);
		}
	private:
		const double T1, T2;
};

class SampleSig:
	public SimObject
{
	public:
		SampleSig(double q1, double q2):
			SimObject(),q1(q1),q2(q2)
		{
		}
		double getQ1() const { return q1; }
		double getQ2() const { return q2; }
		SimObject* clone() const 
		{
			return new SampleSig(q1,q2);
		}
	private:
		const double q1, q2;
};

class NetworkData:
	public SimObject
{
	public:
		enum net_data_t { APP_DATA = 0, TX_CMPLT = 1,
			TX_FAIL = 2, TX_START = 3 };
		NetworkData(net_data_t type, 
				int addr = 0, int bytes = 64,
				SimObject* payload = NULL):
			type(type),addr(addr),bytes(bytes),payload(payload){}
		SimObject* getPayload() { return payload; }
		net_data_t getType() const { return type; }
		void setType(net_data_t newType) { type = newType; }
		int getAddr() const { return addr; }
		void setAddr(int newAddr) { addr = newAddr; }
		SimObject* clone() const
		{
			if (payload == NULL)
				return new NetworkData(type,addr,bytes);
			else
				return new NetworkData(type,addr,bytes,payload->clone());
		}
		int getBytes() const { return bytes; }
		void setBytes(int newBytes) { bytes = newBytes; }
		~NetworkData() { if (payload != NULL) delete payload; }
	private:
		net_data_t type;
		int addr, bytes;
		SimObject* payload;
};

#endif
