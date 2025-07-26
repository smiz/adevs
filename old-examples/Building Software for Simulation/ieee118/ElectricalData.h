#ifndef _electrical_data_h_
#define _electrical_data_h_
#include <complex>
#include <vector>
#include "AdmittanceNetwork.h"
#include "events.h"

/**
 * This inteface class is used to access electrical data for the model.
 * The electrical model consists of buses, or nodes, labeled
 * from 0 to N-1, where N is the number of nodes. All units
 * are assumed to be compatible with the "p.u." system.
 */
class ElectricalData {
  public:
    /**
		 * Structure that describes a transmission line.
		 */
    struct line_t {
        /// Default constructor
        line_t() : from(0), to(0), y() {}
        /**
			 * Create a line from not "from" to "to" with admittance y.
			 */
        line_t(unsigned from, unsigned to, Complex y)
            : from(from), to(to), y(y) {}
        /// Nodes connected by the line
        unsigned from, to;
        /// Admittance of the line
        Complex y;
    };
    /**
		 * Structure that characterizes a synchronous machine.
		 */
    struct genr_t {
        /// The constructor sets some a reasonable (maybe) initial condition
        genr_t()
            : Tspd1(20.0),
              Tspd2(20.0),
              Ef_max(5.0),
              Xd(0.001, 0.01),
              M(3.0),
              R(50.0),
              Te(1.0),
              Vref(1.0),
              Agc(200.0),
              FreqTol(1.0),
              w0(0.0),
              Ef0(1.0),
              T0(0.0),
              Pm0(1.0),
              C0(0.0),
              Ps(1.0),
              fix_at_Pm0(true) {}
        /// Speed governor time constants
        double Tspd1, Tspd2;
        /// Maximum excitation voltage
        double Ef_max;
        /// Machine reactance
        Complex Xd;
        /// Inertia constant
        double M;
        /// Droop
        double R;
        /// Time constant for the exicitation contoller
        double Te;
        /// Voltage magnitude reference
        double Vref;
        /// Area control gain for this machine
        double Agc;
        /// Frequency tolerance (Hertz)
        double FreqTol;
        /// Initial angular velocity (p.u.)
        double w0;
        /// Initial exiciter field voltage (p.u.)
        double Ef0;
        /// Initial power angle (radians)
        double T0;
        /// Initial mechanical power output (p.u.)
        double Pm0;
        /// Initial governor signal
        double C0;
        /// Power set point
        double Ps;
        /**
			 *  If true, the simulator will always use
			 *  Pm0 for the setpoint. This flag is 
			 *  included for backwards compatibility
			 */
        bool fix_at_Pm0;
    };

    /// Constructor
    ElectricalData() {}
    /// Get the number of generators in the model
    virtual unsigned getGenrCount() = 0;
    /// Get the total number of nodes in the model
    virtual unsigned getNodeCount() = 0;
    /// Get the list of nodes that are generation points
    virtual std::vector<unsigned> const &getGenrs() = 0;
    /// Get the list of electrical lines
    virtual std::vector<line_t> const &getLines() = 0;
    /** 
		 * Get the Norton equilvalent current at a load node.
		 * This should be zero except for active loads.
		 */
    virtual Complex getCurrent(unsigned node) = 0;
    /**
		  * Get the Norton equivalent admittance at a load node.
		  * If there is no load, then this should return zero.
		  */
    virtual Complex getAdmittance(unsigned node) = 0;
    /// Get the parameters for a synchronous machine
    virtual genr_t getGenrParams(unsigned genr) = 0;
    /**
		 * Construct an admittance network for this data
		 * and store it in the supplied Ybus which must have
		 * the appropriate size.
		 */
    void buildAdmitMatrix(AdmittanceNetwork &Y);
    /// Destructor
    virtual ~ElectricalData() {}
};

#endif
