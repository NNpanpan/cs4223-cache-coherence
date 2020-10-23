#ifndef CCACHE_H
#define CCACHE_H

#include <string>
#include <utility>
#include <vector>

#include "Bus.h"
#include "BusUser.h"

using namespace std;

/*
Class Cache:
    - Represents the L1 data cache.
    - Is a LRU set-assoc cache.
*/

class Cache : public BusUser {
private:
    int ID;

    int hitNum;
    int totalRq;
    int privateHits;
    int sharedHits;

    int associativity;
    int blockSize;
    int cacheSize;
    int setCount;
    vector<vector<pair<string, vector<int>>>> entries; // cache entries
    vector<int> lastUsed; // last used block for each set

public:
    Cache(const int & assoc, const int & blockSize, const int & cacheSize, const int & ID);

    int getID() const;

    void incrHit();
    int getHitNum();
    void incrTotalRq();
    int getTotalRq();
    void incrPrivateHits();
    int getPrivateHits();
    void incrSharedHits();
    int getSharedHits();
    int getBlockWordCount();

    void setLastUsed(const int & addr);

    
    void allocEntry(const int & addr);
    int hasEntry(const int & addr) const override;
    
    int updateState(const int & addr, string newState);

    virtual int prRd(const int & addr); 
    virtual int prWr(const int & addr);
    int flush(const int & addr) override;
};

#endif
