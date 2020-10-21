#include "Cache.h"
#include "MESICache.h"

MESICache::MESICache(const int & assoc, const int & blockSize, const int & cacheSize) : 
Cache(assoc, blockSize, cacheSize) {}

void MESICache::setMBus(MESIBus* bus_ptr) {
    mBus = bus_ptr;
}

int MESICache::prRd(const int & addr) {
    if (hasEntry(addr)) {
        setLastUsed(addr);
        return 1;
    } else {
        // Search for stuff
        int cost = 0;
        if (mBus->busRd(addr)) {
            // Cache block exists somewhere
            cost = 2 * getBlockWordCount();
            allocEntry(addr);
            updateState(addr, "S");
            setLastUsed(addr);
        } else {
            // Cache block is only in memory
            cost = 100;
            allocEntry(addr);
            updateState(addr, "E");  
            setLastUsed(addr);
        }

        return cost;
    }
    
    return 0;
}