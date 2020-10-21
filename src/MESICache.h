#ifndef MESICACHE_H
#define MESICACHE_H

#include "Cache.h"
#include "MESIBus.h"

/*
Class MESICache:
    - Implements a cache that follows MESI protocol
 */ 

class MESICache : public Cache {
private:
    MESIBus *mBus;
public:
    MESICache(const int & assoc, const int & blockSize, const int & cacheSize);

    void setMBus(MESIBus* bus_ptr);

    int prRd(const int & addr) override;
    int prWr(const int & addr) override;
};

#endif