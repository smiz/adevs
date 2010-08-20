#ifndef Events_h_
#define Events_h_

// Enumeration of simulation event types.
typedef enum { 
	SIM_MOTOR_VOLTAGE, // Tank motor voltage
	SIM_TANK_POSITION, // Tank position
	SIM_INTERRUPT, // Interrupt handler start/end
	SIM_PACKET, // Packet from the network
	SIM_MOTOR_ON_TIME, // Motor on time
	SIM_NO_EVENT // No assigned type
} SimEventType; 
// Motor voltage data
struct SimMotorVoltage { double el, er; };
// Tank position data
struct SimTankPosition { double x, y, theta; };
// On-time settings for the interrupt handler.
struct SimMotorOnTime
{
	unsigned char left, right;
	bool reverse_left, reverse_right;
};
// A network packet
struct SimPacket { float left_power, right_power; };
// The I/O type for all models in the tank simulator
class SimEvent
{
public:
	SimEvent():type(SIM_NO_EVENT){}
	// Create a specific type of event
	SimEvent(SimEventType type):type(type){}
	// Create a SIM_MOTOR_VOLTAGE event
	SimEvent(SimMotorVoltage event):
		type(SIM_MOTOR_VOLTAGE) { data.volts = event; }
	// Create a SIM_TANK_POSITION event
	SimEvent(SimTankPosition event):
		type(SIM_TANK_POSITION) { data.pos = event; }
	// Create an event to set the motor on-time counters
	SimEvent(SimMotorOnTime event):
		type(SIM_MOTOR_ON_TIME) { data.ontime = event; }
	// Create a SIM_PACKET event
	SimEvent(SimPacket event):type(SIM_PACKET) { data.packet = event; }
	// Get the event type
	SimEventType getType() const { return type; }
	// Get the motor voltage data
	const SimMotorVoltage& simMotorVoltage() const { return data.volts; }
	// Get the tank position data
	const SimTankPosition& simTankPosition() const { return data.pos; }
	// Get the motor settings
	const SimMotorOnTime& simMotorOnTime() const { return data.ontime; }
	// Get the network packet
	const SimPacket& simPacket() const { return data.packet; }
	// The STL needs this operator
	bool operator<(const SimEvent& b) const { return type < b.type; }
private:
		SimEventType type;
		union 
		{
			SimMotorVoltage volts;
			SimTankPosition pos;
			SimMotorOnTime ontime;
			SimPacket packet;
		} data;
};

#endif
