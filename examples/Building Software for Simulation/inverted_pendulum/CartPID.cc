#include "CartPID.h"
#include "ByteArrayMessage.h"
#include "MACAddress.h"
#include "Ieee802Ctrl_m.h"
#include <string>

// Network addresses for the OMNET++ network model
static const MACAddress SENSOR_ADDR("999999999999"),
			 CONTROL_ADDR("888888888888");
static const int SAP = 9999; // OMNET++ application ID for the cart/PID
Define_Module(CartPID); // Make CartPID into an OMNET++ module

// OMNET++ calls this method at the start of the simulation
void CartPID::initialize()
{
	// Create the cart and controller
	pid = new PIDControl();
	CartModel *cart = new CartModel();
	hysim = new adevs::Hybrid<double>(
			cart,
			new adevs::corrected_euler<double>(cart,1E-8,0.001),
			new adevs::linear_event_locator<double>(cart,1E-10)
		);
	traj = new TrajRecorder(cart,hysim);
	top_model = new adevs::SimpleDigraph<double>();
	// Models are not coupled because communication is through the
	// OMNET++ model of the Ethernet.
	top_model->add(hysim); top_model->add(pid); 
	sim = new adevs::Simulator<double>(top_model);
	sim->addEventListener(traj);
	sim->addEventListener(this);
	// Schedule first internal event
	if (sim->nextEventTime() < DBL_MAX)
		scheduleAt(SimTime(sim->nextEventTime()),&self_event);
	// Register with the OMNET LLC
	registerDSAP("sensorOut");
	registerDSAP("controlOut");
}

void CartPID::registerDSAP(const char* gate_name)
{
	Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
	etherctrl->setDsap(SAP);
	cMessage *msg =
		new cMessage("register_DSAP", IEEE802CTRL_REGISTER_DSAP);
	msg->setControlInfo(etherctrl);
	send(msg,gate_name);
}

// OMNET++ calls this method when an event occurs at the CartPID model.
// These can be self-scheduled events or the arrival of a message
// from the network.
void CartPID::handleMessage(cMessage *msg)
{
	SimTime timestamp = msg->getArrivalTime();
	// Internal event
	if (msg == &self_event) sim->execNextEvent();
	// External event
	else {
		// Cancel any pending self events
		if (self_event.isScheduled()) cancelEvent(&self_event);
		// Convert to the expected message type
		ByteArrayMessage *data = dynamic_cast<ByteArrayMessage*>(msg);
		assert(data != NULL);
		// Get the data from the message
		adevs::Event<double> x;
		data->copyDataToBuffer(&(x.value),sizeof(double));
		// Inject a sensor reading into the controller
		if (std::string(data->getArrivalGate()->getBaseName()) == "sensorIn") 
			x.model = pid;
		// Control data goes to the cart
		else x.model = hysim;
		// Clean up the message
		delete data;
		// Inject the event into the simulator
		xbag.insert(x);
		sim->computeNextState(xbag,timestamp.dbl());
		xbag.clear();
	}
	// Process instantaneous responses to the input
	while (SimTime(sim->nextEventTime()) <= timestamp) 
		sim->execNextEvent();
	// Schedule the next internal event
	if (sim->nextEventTime() < DBL_MAX)
		scheduleAt(SimTime(sim->nextEventTime()),&self_event);
}

// This method is called by our simulator whenever the cart or
// controller produces an output event.
void CartPID::outputEvent(adevs::Event<double> y, double t)
{
	Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
	etherctrl->setSsap(SAP);
	etherctrl->setDsap(SAP);
	// Sensor output; send it to the controller
	if (y.model == hysim) {
		etherctrl->setDest(CONTROL_ADDR);
		ByteArrayMessage *msg =
			new ByteArrayMessage("Sensor_data",IEEE802CTRL_DATA);
		msg->setControlInfo(etherctrl);
		msg->setDataFromBuffer(&(y.value),sizeof(double));
		send(msg,"sensorOut");
	}
	// Control output; send it to the sensor
	else {
		etherctrl->setDest(SENSOR_ADDR);
		ByteArrayMessage *msg =
			new ByteArrayMessage("Control_data",IEEE802CTRL_DATA);
		msg->setControlInfo(etherctrl);
		msg->setDataFromBuffer(&(y.value),sizeof(double));
		send(msg,"controlOut");
	}
}

CartPID::~CartPID()
{
	delete sim; delete top_model; delete traj;
}
