#ifndef ETHERNET_MODEL_H_
#define ETHERNET_MODEL_H_
#include "adevs.h"
#include "io.h"
#include <list>

class Rx:
	public AtomicModel
{
	public:

		// Positive numbers are delivered to apps
		static const int from_media;
		static const int to_app;

		Rx(int addr);
		void delta_int();
		void delta_ext(double e, const adevs::Bag<IO_Type>& xb);
		void delta_conf(const adevs::Bag<IO_Type>& xb);
		void output_func(adevs::Bag<IO_Type>& yb);
		double ta();
		~Rx();
	private:
		const int addr;
		NetworkData* data;		
};

class Tx:
	public AtomicModel
{
	public:

		static const int to_media;
		static const int from_media;
		static const int from_app;

		Tx(int maxTries);
		void delta_int();
		void delta_ext(double e, const adevs::Bag<IO_Type>& xb);
		void delta_conf(const adevs::Bag<IO_Type>& xb);
		void output_func(adevs::Bag<IO_Type>& yb);
		double ta();
		int getCollisions() const { return collisions; }
		~Tx();
	private:
		enum Mode { BACKOFF, FAIL, START, TRANSMIT, IDLE };
		Mode mode;
		list<NetworkData*> q;	
		bool busy;
		int tryCount;
		const int maxTries;
		double timeToTx;
		int collisions;
};

/**
 * This card sends and receives to its own address.
 * This works only if a communicating pair have the
 * same address.
 */
class NetworkCard:
	public adevs::Digraph<SimObject*>
{
	public:

		static const int to_media;
		static const int from_media;
		static const int to_app;
		static const int from_app;

		NetworkCard(int addr, int maxTries = 16);
		int getCollisions() const { return tx->getCollisions(); }
	private:
		Tx* tx;
};

/**
 * A network to which cards can be attached.
 */
class Ethernet:
	public adevs::Digraph<SimObject*>
{
	public:
		Ethernet();
		// Returns the ports for attaching an application to the card
		void attach(NetworkCard* card, int& to_app, int& from_app);
	private:
		std::vector<NetworkCard*> cards;
};

#endif
