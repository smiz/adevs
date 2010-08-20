#ifndef _TankPhysics_h_
#define _TankPhysics_h_
#include "adevs.h"
#include "TankPhysicsEqns.h"
#include "SimEvents.h"
#include <fstream>

class TankPhysics:
	public adevs::Hybrid<SimEvent>
{
	public:
		/// Create a tank that will use the supplied physics object
		TankPhysics(TankPhysicsEqns* eqns);
		/// Destructor
		~TankPhysics(){}
		/// Get the left motor current
		double leftMotorCurrent() const;
		/// Get the right motor current
		double rightMotorCurrent() const;
		/// Get the tank's linear speed
		double getSpeed() const;
		/// Get the tank's rotational speed
		double getTurnSpeed() const;
		/// Get the x coordinate of the tank
		double getX() const;
		/// Get the y coordinate of the tank
		double getY() const;
		/// Get the heading of the tank
		double getHeading() const;
		/// Is the tank turning?
		bool isTurning() const;
		/// Get the torque
		double getTorque() const;
		/// Get the left track force
		double getLeftForce() const;
		/// Get the right track force
		double getRightForce() const;
		/// Get the left motor speed
		double leftMotorSpeed() const;
		/// Get the right motor speed
		double rightMotorSpeed() const;
		/// Get the resistance of the motor (left and right are the same)
		double getMotorOhms() const { return eqns->getMotorOhms(); }
	private:
		TankPhysicsEqns* eqns;
};

#endif
