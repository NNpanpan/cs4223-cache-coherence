#include <string>
#include <utility>
#include <vector>
#include <cassert>

#include "Cache.h"

using namespace std;


/*******************************
 * Cache entry implementation
 *******************************/


CacheEntry::CacheEntry(string state,
    int lastUsed, int blockNumber, int validFrom) {
    this->state = state;
    this->lastUsed = lastUsed;
    this->blockNumber = blockNumber;
    this->validFrom = validFrom;
}
string CacheEntry::getState() {
    return state;
}
int CacheEntry::getLastUsed() {
    return lastUsed;
}
int CacheEntry::getBlockNumber() {
    return blockNumber;
}

bool CacheEntry::isInvalid() {
    return getState() == "I";
}

bool CacheEntry::isPrivate() {
    string state = getState();
    if (state == "M") return true; /// modified
    return false;
}

void CacheEntry::setLastUsed(int lastUsed) {
    this->lastUsed = lastUsed;
}
void CacheEntry::setState(string state) {
    this->state = state;
}

void CacheEntry::setValidFrom(int validFrom) {
    this->validFrom = validFrom;
}

int CacheEntry::getValidFrom() {
    return validFrom;
}


/**************************************
 * Cache implementation
 **************************************/
Cache::Cache(int assoc, int blockSize, int cacheSize, int ID) {
    this->associativity = assoc;
    this->blockSize = blockSize;
    this->cacheSize = cacheSize;

    setCount = cacheSize / (blockSize * assoc);
    this->entries = vector<vector<CacheEntry>>(setCount, vector<CacheEntry>(associativity, CacheEntry()));
    this->ID = ID;
}


/// private func

int Cache::getCacheIndex(int blockNumber) {
    return blockNumber % setCount;
}

int Cache::getBlockNumber(int addr) {
    return addr / blockSize;
}

int Cache::getAssocNumber(int addr) {
    int blockNumber = getBlockNumber(addr);
    int cacheIndex = getCacheIndex(blockNumber);
    vector<CacheEntry> &currentSet = entries[cacheIndex];
    for(int num = 0; num < currentSet.size(); num++) {
        CacheEntry entry = currentSet[num];
        if (entry.isInvalid()) {
            continue;
        }
        if (entry.getBlockNumber() == blockNumber) {
            ///same block number
            return num;
        }
    }
    return -1;
}

int Cache::getEvictedAssocNumber(int cacheIndex) {
    vector<CacheEntry> &currentSet = entries[cacheIndex];
    /// if there is an invalid entry, choose it
    for(int num = 0; num < currentSet.size(); num++) {
        CacheEntry entry = currentSet[num];
        if (entry.isInvalid()) {
            return num;
        }
    }

    ///otherwise, LRU policy
    int result = 0;
    for(int num = 0; num < currentSet.size(); num++) {
        CacheEntry entry = currentSet[num];
        if (entry.getLastUsed() < currentSet[result].getLastUsed())
            result = num;
    }
    return result;
}

CacheEntry& Cache::getEntry(int addr) {
    assert(hasEntry(addr));
    int blockNumber = getBlockNumber(addr);
    int cacheIndex = getCacheIndex(blockNumber);
    int assocNumber = getAssocNumber(addr);
    return entries[cacheIndex][assocNumber];
}

/// public func
int Cache::getID() {
    return ID;
}

void Cache::setBlockLastUsed(int addr, int lastUsed) {
    getEntry(addr).setLastUsed(lastUsed);
}

void Cache::setBlockValidFrom(int addr, int validFrom) {
    getEntry(addr).setValidFrom(validFrom);
}

void Cache::setBlockState(int addr, string state) {
    getEntry(addr).setState(state);
}

string Cache::getBlockState(int addr) {
    return getEntry(addr).getState();
}

CacheEntry Cache::evictEntry(int addr) {
    assert(!hasEntry(addr));
    int blockNumber = getBlockNumber(addr);
    int cacheIndex = getCacheIndex(blockNumber);
    int evictedAssocNumber = getEvictedAssocNumber(cacheIndex);
    CacheEntry result = entries[cacheIndex][evictedAssocNumber];
    entries[cacheIndex][evictedAssocNumber] = CacheEntry();
    return result;
}
void Cache::allocEntry(int addr, string state, int lastUsed, int validFrom) {
    assert(!hasEntry(addr));
    int blockNumber = getBlockNumber(addr);
    int cacheIndex = getCacheIndex(blockNumber);
    int evictedAssocNumber = getEvictedAssocNumber(cacheIndex);

    assert(entries[cacheIndex][evictedAssocNumber].isInvalid());

    entries[cacheIndex][evictedAssocNumber] = CacheEntry(state, lastUsed, blockNumber, validFrom);
}

int Cache::hasEntry(int addr) {
    return getAssocNumber(addr) != -1;
}

bool Cache::isAddrPrivate(int addr) {
    assert(hasEntry(addr));
    return getEntry(addr).isPrivate();
}

bool Cache::isAddrInvalid(int addr) {
    if (!hasEntry(addr)) return true;
    return getEntry(addr).isInvalid();
}

int Cache::getAddrUsableTime(int addr) {
    assert(hasEntry(addr));
    return getEntry(addr).getValidFrom();
}

int Cache::getHeadAddr(CacheEntry entry) {
    int blockNumber = entry.getBlockNumber();
    return blockNumber * blockSize;
}



