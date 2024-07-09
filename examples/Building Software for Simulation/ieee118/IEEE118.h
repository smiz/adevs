#ifndef IEEE118_h_
#define IEEE118_h_
#include "IEEE_CDF_Data.h"

/**
 * This creates an instance of a 118 bus network
 * from ieee118cdf.txt and sets its initial
 * conditions using data from ieee118cdf.ini
 * The initial data was produced using the cdf
 * example from THYME-1.0.3
 */
class IEEE118 : public IEEE_CDF_Data {
  public:
    IEEE118();

  private:
    void initialize();
};

#endif
