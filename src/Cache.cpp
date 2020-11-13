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

bool CacheEntry::isModified() {
    return state == "M";
}

bool CacheEntry::isPrivate() {
    string state = getState();
    return (state == "E" || state == "M");
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
    this->entries = vector<vector<CacheEntry>>(setCount,
        vector<CacheEntry>(associativity, CacheEntry()));
    this->ID = ID;
}

// Private functions

int Cache::getCacheIndex(int blockNumber) {
    return blockNumber % setCount;
}

int Cache::getAssocNumber(int addr) {
    int blockNumber = getBlockNumber(addr);
    int cacheIndex = getCacheIndex(blockNumber);
    vector<CacheEntry> &currentSet = entries[cacheIndex];

    for(int num = 0; num < associativity; num++) {
        CacheEntry entry = currentSet[num];
        if (!entry.isInvalid() && entry.getBlockNumber() == blockNumber) {
            // Valid entry exists with matching block number
            return num;
        }
    }
 
    return -1;
}

int Cache::getEvictedAssocNumber(int cacheIndex) {
    vector<CacheEntry> &currentSet = entries[cacheIndex];
 
    // Select invalid entry if any exist
    for(int num = 0; num < associativity; num++) {
        CacheEntry entry = currentSet[num];
        if (entry.isInvalid()) {
            return num;
        }
    }

    // Otherwise use LRU policy to select oldest block
    int result = 0;
    for(int num = 0; num < associativity; num++) {
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

// Public functions

int Cache::getBlockNumber(int addr) {
    return addr / blockSize;
}

int Cache::getID() {
    return ID;
}

int Cache::hasEntry(int addr) {
    return getAssocNumber(addr) != -1;
}

int Cache::getHeadAddr(CacheEntry entry) {
    int blockNumber = entry.getBlockNumber();
    return blockNumber * blockSize;
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

bool Cache::isAddrDirty(int addr) {
    return getEntry(addr).isModified();
}

bool Cache::isAddrPrivate(int addr) {
    return getEntry(addr).isPrivate();
}

bool Cache::isAddrInvalid(int addr) {
    if (!hasEntry(addr)) return true;
    return getEntry(addr).isInvalid();
}

int Cache::getAddrUsableTime(int addr) {
    return getEntry(addr).getValidFrom();
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

    entries[cacheIndex][evictedAssocNumber] = CacheEntry(state, lastUsed,
        blockNumber, validFrom);
}

