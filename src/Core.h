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

    // Instruc Q: Either maintains a file pointer, or read all
    int isFree;
    int nextFreeCycle;
    Cache* cache;

    Core(const Cache* & cache_ptr);
    int computeOthers(const int & cycles) const;
    int prRd(const int & addr) const;
    int prWr(const int & addr) const;
};

#endif