#ifndef CORE_H
#define CORE_H

#include "Cache.h"

/*
Class Core:
    - Represents the processor
*/

class Core {
private:
    int ID;

    int execCycles;
    int compCycles;
    int idleCycles;
    int loadStoreInstCount;

    // Instruc Q is external
    int isFree;
    int nextFreeCycle;
    Cache* cache;

public:
    Core(Cache* cache_ptr, const int & id);

    int getID() const;

    void setExecCycles(const int & cycles);
    void setCompCycles(const int & cycles);
    void setIdleCycles(const int & cycles);
    void setLSInstCount(const int & cnt);

    void incrExecCycles(const int & cycles);
    int getExecCycles();
    void incrCompCycles(const int & cycles);
    int getCompCycles();
    void incrIdleCycles(const int & cycles);
    int getIdleCycles();
    void incrLSInstCount();
    int getLSInstCount();

    void setIsFree(const int & val);
    int getIsFree();
    void setNextFreeCycle(const int & cycle);

    
    int execCmd(const int & cmdType, const int & info);

    int computeOthers(const int & cycles);
    int prRd(const int & addr);
    int prWr(const int & addr);

    void finaliseStats();
};

#endif
