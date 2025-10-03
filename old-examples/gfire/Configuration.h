#ifndef _configuration__h_
#define _configuration__h_
#include <cmath>
#include <fstream>
#include <string>
#include "fireCell.h"

/**
Load and provide access to initialization data.
*/
class Configuration {
  public:
    // Load the specified configuration data file
    Configuration(std::string config_file);
    // Get the width of the cellspace
    int get_width() const { return width; }
    // Get the height of the cellspace
    int get_height() const { return height; }
    // Get the amount of fuel at location x,y
    double get_fuel(int x, int y) const { return fuel[x][y]; }
    // True indicates that x,y is initial burning
    bool get_fire(int x, int y) const { return fire[x][y]; }
    // Destructor
    ~Configuration();

  private:
    void load_fire();
    void load_fuel();
    std::ifstream data;
    int width, height;
    double** fuel;
    bool** fire;
};

#endif
