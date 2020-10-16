#ifndef CORE_H
#define CORE_H

#include <fstream>
#include <string>

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

    // Instruc Q
    std::ifstream ifs;
    int isFree;
    int nextFreeCycle;
    Cache* cache;

    Core(const string & filepath, Cache* cache_ptr);
    ~Core();

    int incrCycles(const int & cycles);

    int computeOthers(const int & cycles);
    int prRd(const int & addr);
    int prWr(const int & addr);

    void finaliseStats();
};

#endif