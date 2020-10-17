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

class Cache {
public:
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
    BusUser* busUser;

    Cache(const int & assoc, const int & blockSize, const int & cacheSize);
    
    void allocEntry(const int & addr);
    int hasEntry(const int & addr) const;
    
    void updateState(const int & addr, string newState);
    void setBusUser(BusUser* busUser);
    int prRd(const int & addr) const; // calls busRd on busUser
    int prWr(const int & addr) const; // calls busRdX or busUpdate on busUser
    int flush(const int & addr) const; // calls blush on busUser
};

#endif