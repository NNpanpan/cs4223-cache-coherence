#ifndef CCACHE_H
#define CCACHE_H

#include <string>
#include <utility>
#include <vector>

using namespace std;

class CacheEntry {
private:
    string state;
    int lastUsed;
    int blockNumber;
    int validFrom;
public:
    CacheEntry(string state = "I",
        int lastUsed = -1, int blockNumber = -1, int validFrom = -1);
    string getState();
    int getLastUsed();
    int getBlockNumber();

    bool isInvalid();
    bool isPrivate();

    void setLastUsed(int lastUsed);
    void setState(string state);
    void setValidFrom(int validFrom);

    int getValidFrom();
};

/*
Class Cache:
    - Represents the L1 data cache.
    - Is a LRU set-assoc cache.
*/

class Cache {
private:
    int ID;
    int associativity;
    int blockSize;
    int cacheSize;
    int setCount;

    ///first dimension size = number of set, 2nd dimension is associativity
    ///assume no memory cost for storing block state and block number
    vector<vector<CacheEntry>> entries; // cache entries, state + last used

    int getCacheIndex(int blockNumber); /// 0 to setCount - 1


    ///where is the block that contain this addr in the set, -1 if not contain
    int getAssocNumber(int addr);
    ///where is the block that will be evicted for a cache set
    int getEvictedAssocNumber(int cacheIndex);
    CacheEntry& getEntry(int addr);
public:

    int getBlockNumber(int addr);

    Cache(int assoc, int blockSize, int cacheSize, int ID);
    int getID();

    /// the block contain this address must be inside the cache
    void setBlockLastUsed(int addr, int lastUsed);
    void setBlockValidFrom(int addr, int validFrom);
    void setBlockState(int addr, string newState);
    string getBlockState(int addr);

    /// evict and return the evicted entry belong to same set as addr (can be invalid)
    CacheEntry evictEntry(int addr);
    /// alloc entry, must have call evictEntry before
    void allocEntry(int addr, string state, int lastUsed, int validFrom);
    /// only consider to has IFF not invalid
    int hasEntry(int addr);

    /// must have in cache, return true if in M state, otherwise false
    bool isAddrPrivate(int addr);
    /// return true if (not in cache or I state), otherwise false
    bool isAddrInvalid(int addr);

    /// return the cycle when the addr is usable, must currently in cache
    int getAddrUsableTime(int addr);

    /// return head address of the entry
    int getHeadAddr(CacheEntry entry);
};

#endif
