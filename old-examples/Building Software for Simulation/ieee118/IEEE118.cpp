#include "IEEE118.h"
#include <fstream>
#include <iostream>
#include <vector>


IEEE118::IEEE118() : IEEE_CDF_Data("ieee118cdf.txt") {
    initialize();
}

void IEEE118::initialize() {
    ifstream fin("ieee118cdf.ini");
    double t;
    fin >> t;
    int breaker;
    for (std::vectorunsigned>::const_iterator iter = getGenrs().begin();
         iter != getGenrs().end(); iter++) {
        genr_t p = getGenrParams(*iter);
        fin >> p.w0 >> p.T0 >> p.C0 >> p.Pm0 >> p.Ef0 >> breaker;
        // These parameters create an underdamped system
        //		p.M = 3.0;
        p.M = 3.0;
        p.Ef_max = 5.0;
        //		p.FreqTol = 1.25;
        p.FreqTol = 0.0025;
        //		p.FreqTol = (1.25/60.0)/(6.28);
        p.Ps = 10.0;
        p.fix_at_Pm0 = false;
        p.Tspd1 = 20;
        p.Tspd2 = 20;
        p.R = 50.0;
        p.Te = 0.01;
        setGenrParams(*iter, p);
    }
}
