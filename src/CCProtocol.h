#ifndef CCPROTOCOL_H
#define CCPROTOCOL_H

#include "Cache.h"

/*
Class CCProtocol:
    - Interface for cache coherence protocols
*/

class CCProtocol {
public:
    virtual void nextState(const Cache* cache) = 0;
    virtual void prRd(const Cache* cache, const int & addr) = 0;
    virtual void prWr(const Cache* cache, const int & addr) = 0;
};

#endif