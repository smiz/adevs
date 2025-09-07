#include "ElectricalData.h"
#include <iostream>


void ElectricalData::buildAdmitMatrix(AdmittanceNetwork &Y) {
    for (std::vectorline_t>::const_iterator iter = getLines().begin();
         iter != getLines().end(); iter++) {
        int from = (*iter).from;
        int to = (*iter).to;
        Y.add_line(from, to, (*iter).y);
    }
    for (std::vectorunsigned>::const_iterator iter = getGenrs().begin();
         iter != getGenrs().end(); iter++) {
        Y.add_self(*iter, 1.0 / getGenrParams(*iter).Xd);
    }
    for (unsigned i = 0; i < getNodeCount(); i++) {
        Y.add_self(i, getAdmittance(i));
    }
}
