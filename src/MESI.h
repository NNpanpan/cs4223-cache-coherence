#ifndef MESICC_H
#define MESICC_H

#include "CCProtocol.h"

/*
Class MESI:
    - Implements MESI cache coherence protocol
*/

class MESI : public CCProtocol {
public:
    void nextState(const Cache* cache);
    void prRd(const Cache* cache, const int & addr);
    void prWr(const Cache* cache, const int & addr);
};

#endif