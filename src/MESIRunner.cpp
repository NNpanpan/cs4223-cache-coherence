#include "MESIRunner.h"

MESIRunner::MESIRunner(int cacheSize, int assoc, int blockSize,
    vector<vector<pair<int, int>>> coreTraces)
    : Runner(cacheSize, assoc, blockSize, coreTraces) {
}

bool MESIRunner::cacheAllocAddr(int cacheID, int addr, string addrState) {
    // Fetch a block from mem, allocate a line, evicting another block if needed
    Cache& cache = caches[cacheID];
    int blockNum = cache.getBlockNumber(addr);

    // Data only available from memory after additional 100 cycles
    int availableTime = getMemBlockAvailableTime(blockNum) + 100;

    // Pre-condition: find an available line (evict one if none available)
    CacheEntry evictedEntry = cache.evictEntry(addr);
    // If entry is valid, this is the victim block (cache set conflict)
    if (!evictedEntry.isInvalid()) {
        // Perform write-back to update mem if victim block in 'M' state
        if (evictedEntry.getState() == "M") {
            int evictedAddr = cache.getHeadAddr(evictedEntry);
            cacheWriteBackMem(cacheID, evictedAddr);
            availableTime += 100;   // Need 100 cycles to write-back this block
        }
    }

    cache.allocEntry(addr, addrState, curTime, availableTime);
    return true;
}

void MESIRunner::invalidateO(int cacheID, int addr, bool needWriteBack) {
    // Invalidate all other copies in other caches
    for(int othCacheID = 0; othCacheID < (int) caches.size(); othCacheID++) {
        if (othCacheID == cacheID) continue;
        Cache& othCache = caches[othCacheID];

        if (othCache.hasEntry(addr)) {
            // If copy is dirty ('M' state) - only one process holds line
            if (othCache.isAddrPrivate(addr) && needWriteBack) {
                cacheWriteBackMem(othCacheID, addr);
            }

            // Otherwise immediately invalidate that entry, whatever its state
            othCache.setBlockState(addr, "I");

            // Update stat 7
            bus.incInvalidateCount();
        }
    }
}

void MESIRunner::simulateReadHit(int coreID, int addr) {
    Cache& cache = caches[coreID];
    cache.setBlockLastUsed(addr, curTime);
}

void MESIRunner::simulateWriteHit(int coreID, int addr) {
    int cacheID = coreID;
    Cache& cache = caches[cacheID];
    string blockState = cache.getBlockState(addr);

    cache.setBlockLastUsed(addr, curTime);
    cache.setBlockState(addr, "M");

    // Should have no other copies in 'M'/'E' so this will only invalidate them
    // No write-back needed
    invalidateO(cacheID, addr, false);

    // Mem does not hold an updated copy
    int blockNum = cache.getBlockNumber(addr);
    invalidBlock[blockNum] = INF;
}

void MESIRunner::simulateReadMiss(int coreID, int addr) {
    int cacheID = coreID;

    // Check if any cache holds modified address
    for(int othCacheID = 0; othCacheID < (int) caches.size(); othCacheID++) {
        if (othCacheID == cacheID) continue;

        Cache& othCache = caches[othCacheID];
        if (othCache.hasEntry(addr)) {
            if (othCache.isAddrPrivate(addr)) {
                // Snooped write
                cacheWriteBackMem(othCacheID, addr);
                othCache.setBlockState(addr, "S");
            }
        }
    }

    // Determine if line should transition to 'E' or 'S'
    int countHold = 0;
    for(int othCacheID = 0; othCacheID < (int) caches.size(); othCacheID++) {
        if (othCacheID == cacheID) continue;
        Cache& othCache = caches[othCacheID];
        if (othCache.hasEntry(addr)) {
            countHold++;
        }
    }

    string addrState = (countHold == 0) ? "E" : "S";
    cacheAllocAddr(cacheID, addr, addrState);

    // Update stat 6 + 7
    bus.incTrafficBlock();
}

void MESIRunner::simulateWriteMiss(int coreID, int addr) {
    int cacheID = coreID;
    Cache& cache = caches[cacheID];

    // Invalidate with write-back if another cache a copy in 'M' state
    invalidateO(cacheID, addr, true);

    // Transition to 'M' state
    string addrState = "M";
    cacheAllocAddr(cacheID, addr, addrState);

    // Mme does not hold an updated copy
    int blockNum = cache.getBlockNumber(addr);
    invalidBlock[blockNum] = INF;

    // Update stat 6 + 7
    bus.incTrafficBlock();
}

void MESIRunner::progressTime(int newTime) {
    for(auto &core : cores) {
        core.progress(newTime - curTime);
    }

    curTime = newTime;
    checkMem();
}

MESIRunner::~MESIRunner() {
}

