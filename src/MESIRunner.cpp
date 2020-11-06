#include "MESIRunner.h"

#include <bits/stdc++.h>

using namespace std;

MESIRunner::MESIRunner(int cacheSize, int assoc, int blockSize,
    vector<vector<pair<int, int>>> coreTraces)
    : Runner(cacheSize, assoc, blockSize, coreTraces) {
}

bool MESIRunner::cacheAllocAddr(int cacheID, int addr, string addrState) {
    /// fetch a block from mem, alloc a cache block and evict another block if needed
    Cache& cache = caches[cacheID];

    int blockNum = cache.getBlockNumber(addr);

    /// additional 100 cycles
    int availableTime = getMemBlockAvailableTime(blockNum) + 100;

    /// assuming evict before alloc
    CacheEntry evictedEntry = cache.evictEntry(addr);
    /// if that entry is valid, mean cache set conflict
    if (!evictedEntry.isInvalid()) {
        /// need to rewrite if in "M" state
        if (evictedEntry.getState() == "M") {
            int evictedAddr = cache.getHeadAddr(evictedEntry);
            cacheWriteBackMem(cacheID, evictedAddr);
            availableTime += 100; /// 100 cycles first to evict this addr
        }
    }

    //cout << "Cache " << cacheID << " have " << addr << " at " << availableTime << endl;

    cache.allocEntry(addr, addrState, curTime, availableTime);
    return true;
}

void MESIRunner::invalidateO(int cacheID, int addr, bool needWriteBack) {
    /// invalidate all other caches
    for(int othCacheID = 0; othCacheID < caches.size(); othCacheID++) {
        if (othCacheID == cacheID) continue;
        Cache& othCache = caches[othCacheID];
        if (othCache.hasEntry(addr)) {
            ///if that entry is 'M'
            if (othCache.isAddrPrivate(addr) && needWriteBack) {
                cacheWriteBackMem(othCacheID, addr);
            }
            /// immediately invalidate that entry, regardless of its state
            othCache.setBlockState(addr, "I");
            /// update stat 7
            bus.incInvalidateCount();
        }
    }
}

void MESIRunner::simulateReadHit(int coreID, int addr) {
    //cout << "Core " << coreID << " read hit addr " << addr << " at " << curTime << endl; 
    Cache& cache = caches[coreID];
    cache.setBlockLastUsed(addr, curTime);
}
void MESIRunner::simulateWriteHit(int coreID, int addr) {
    //cout << "Core " << coreID << " write hit addr " << addr << " at " << curTime << endl; 
    int cacheID = coreID;
    Cache& cache = caches[cacheID];
    string blockState = cache.getBlockState(addr);

    cache.setBlockLastUsed(addr, curTime);
    cache.setBlockState(addr, "M");

    /// should have no other cache in 'M' so this will only invalidate them
    invalidateO(cacheID, addr, false);

    /// mem does not hold a copy
    int blockNum = cache.getBlockNumber(addr);
    invalidBlock[blockNum] = INF;
}

void MESIRunner::simulateReadMiss(int coreID, int addr) {
    //cout << "Core " << coreID << " read miss addr " << addr << " at " << curTime << endl; 
    int cacheID = coreID;
    Cache &cache = caches[cacheID];

    /// check if any cache hold modified address
    for(int othCacheID = 0; othCacheID < caches.size(); othCacheID++) {
        if (othCacheID == cacheID) continue;
        Cache& othCache = caches[othCacheID];
        if (othCache.hasEntry(addr)) {
            if (othCache.isAddrPrivate(addr)) {
                /// snooping write
                cacheWriteBackMem(othCacheID, addr);
                othCache.setBlockState(addr, "S");
            }
        }
    }
    /// check if cache should go to 'E' or 'S'
    int countHold = 0;
    for(int othCacheID = 0; othCacheID < caches.size(); othCacheID++) {
        if (othCacheID == cacheID) continue;
        Cache& othCache = caches[othCacheID];
        if (othCache.hasEntry(addr)) {
            countHold++;
        }
    }
    string addrState = (countHold == 0) ? "E" : "S";
    cacheAllocAddr(cacheID, addr, addrState);
}
void MESIRunner::simulateWriteMiss(int coreID, int addr) {
    //cout << "Core " << coreID << " write miss addr " << addr << " at " << curTime << endl; 
    int cacheID = coreID;
    Cache& cache = caches[cacheID];
    invalidateO(cacheID, addr, true);
    /// go to 'M' state
    string addrState = "M";
    cacheAllocAddr(cacheID, addr, addrState);

    /// mem does not hold a copy
    int blockNum = cache.getBlockNumber(addr);
    invalidBlock[blockNum] = INF;
}
void MESIRunner::cacheWriteBackMem(int cacheID, int addr) {
    Cache& cache = caches[cacheID];
    int blockNum = cache.getBlockNumber(addr);
    assert(invalidBlock[blockNum] == INF); /// mem should not hold this address
    invalidBlock[blockNum] = curTime + 100;

    /// update stat 6 + 7
    bus.incUpdateCount();
    bus.incTrafficBlock();
}

void MESIRunner::checkMem() {
    vector<int> unfreezeBlock;
    for(auto ite : invalidBlock) if (ite.second == curTime) {
        unfreezeBlock.push_back(ite.first);
    }
    for(auto block : unfreezeBlock) {
        invalidBlock.erase(block);
    }
}

void MESIRunner::progressTime(int newTime) {
    for(auto &core : cores) {
        core.progress(newTime - curTime);
    }
    curTime = newTime;
    checkMem();
}

int MESIRunner::getMemBlockAvailableTime(int blockNum) {
    auto ite = invalidBlock.find(blockNum);
    if (ite == invalidBlock.end()) {
        return curTime;
    }
    assert(ite->second >= curTime);
    return ite->second;
}

MESIRunner::~MESIRunner() {
}
