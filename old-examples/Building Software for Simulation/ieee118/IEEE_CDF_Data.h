#ifndef _ieee_cdf_data_h_
#define _ieee_cdf_data_h_
#include <exception>
#include <map>
#include "ElectricalData.h"

/**
 * Exception to report problems with the input file.
 */
class IEEE_CDF_FileException : public std::exception {
  public:
    IEEE_CDF_FileException(std::string err_msg)
        : std::exception(), err_msg(err_msg) {}
    char const* what() const throw() { return err_msg.c_str(); }
    ~IEEE_CDF_FileException() throw() {}

  private:
    std::string const err_msg;
};

/**
 * Loads a test case from an IEEE CDF file format. The generator parameters are
 * fixed at ???.
 */
class IEEE_CDF_Data : public ElectricalData {
  public:
    /// Constructor
    IEEE_CDF_Data(char const* data_file);
    /// Get the number of generators in the model
    virtual unsigned getGenrCount() { return genr_nodes.size(); }
    /// Get the total number of nodes in the model
    virtual unsigned getNodeCount() { return nodes.size(); }
    /// Get the list of nodes that are generation points
    virtual std::vector<unsigned> const &getGenrs() { return genr_nodes; }
    /// Get the list of electrical lines
    virtual std::vector<line_t> const &getLines() { return lines; }
    /** 
		 * Get the Norton equilvalent current at a load node.
		 * This should be zero except for active loads.
		 */
    virtual Complex getCurrent(unsigned node);
    /**
		  * Get the Norton equivalent admittance at a load node.
		  * If there is no load, then this should return zero.
		  */
    virtual Complex getAdmittance(unsigned node);
    /// Get the parameters for a synchronous machine
    virtual genr_t getGenrParams(unsigned genr);
    void setGenrParams(unsigned genr, genr_t params);
    /// Destructor
    ~IEEE_CDF_Data();

  private:
    struct bus_data_t {
        int ID;            // bus identifier
        double v;          // bus voltage (pu)
        double theta;      // bus angle (radians)
        double load_mw;    // real load
        double load_mvar;  // imaginary load
        double genr_mw;    // real generation
        double genr_mvar;  // imaginary generation
        double G, B;       // shunt conductance and capacitance
    };
    std::vector<unsigned> genr_nodes;
    std::vector<bus_data_t> nodes;
    std::map<unsigned, genr_t> genrs;
    std::vector<line_t> lines;
    void addGenr(bus_data_t bus_data);
    void setInitialConditions();
    // Load columns from start to end inclusive from line
    // into buffer. First column is at 1.
    static int const LINE_LEN;
    char *line, *buffer;
    void read_field(int start, int end);
};

#endif
