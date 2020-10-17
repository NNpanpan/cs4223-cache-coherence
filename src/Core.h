#ifndef CORE_H
#define CORE_H

#include "Cache.h"

/*
Class Core:
    - Represents the processor
*/

class Core {
public:
    int execCycles;
    int compCycles;
    int idleCycles;
    int loadStoreInstCount;

    // Instruc Q is external
    int isFree;
    int nextFreeCycle;
    Cache* cache;

    Core(Cache* cache_ptr);

    int incrCycles(const int & cycles);
    int execCmd(const int & cmdType, const int & info);

    int computeOthers(const int & cycles);
    int prRd(const int & addr);
    int prWr(const int & addr);

    void finaliseStats();
};

#endif