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
 *  Class Cache:
 *  - Represents the L1 data cache.
 *  - Is a LRU set-assoc cache.
 *  - Assumes cache size excludes cost of storing block state, block number.
 */
class Cache {
private:
    int ID;
    int associativity;
    int blockSize;
    int cacheSize;
    int setCount;

    // First dimension: set index in [0, setCount - 1]
    // Second dimension: (associativity) number of tags of blocks in this set
    vector<vector<CacheEntry>> entries;     // Cache entries, state + last used

    // Returns set index block is mapped to: [0, setCount - 1]
    int getCacheIndex(int blockNumber);

    // Returns index of line containing a block in this set, or -1 if not in set
    //      [0, associativity - 1]
    int getAssocNumber(int addr);
    // Returns index of line containing block to be evicted from this set
    int getEvictedAssocNumber(int cacheIndex);
    CacheEntry& getEntry(int addr);

public:
    int getBlockNumber(int addr);

    Cache(int assoc, int blockSize, int cacheSize, int ID);
    int getID();

    // Returns true if block containing addr is in a line in the mapped set
    int hasEntry(int addr);

    // Returns starting (head) address of a given cache line
    int getHeadAddr(CacheEntry entry);
 
    /*
     *  PRE-CONDITION: block containing addr must be in line inside cache
     */
    void setBlockLastUsed(int addr, int lastUsed);
    void setBlockValidFrom(int addr, int validFrom);
    void setBlockState(int addr, string newState);
    string getBlockState(int addr);

    // Returns true if line containing this block in M state, false otherwise
    bool isAddrPrivate(int addr);
    // Returns true if block not in cache/in cache with I state, false otherwise
    bool isAddrInvalid(int addr);
 
    // Returns time (cycles) when the block containing addr is loaded into cache
    int getAddrUsableTime(int addr);
    /*  END PRE-CONDITION */

    // Pre-condition: block containing addr is not already in the cache
    // Evicts and return the evicted entry from the set the block containing
    // addr is mapped to (can be invalid)
    CacheEntry evictEntry(int addr);
    // Pre-condition: evictEntry called immediately before to ensure free line 
    // Allocates a new entry in a free line in the mapped set
    void allocEntry(int addr, string state, int lastUsed, int validFrom);
};

#endif

