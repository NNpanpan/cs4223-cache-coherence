#include "MESIFRunner.h"

MESIFRunner::MESIFRunner(int cacheSize, int assoc, int blockSize,
    vector<vector<pair<int, int>>> coreTraces)
    : Runner(cacheSize, assoc, blockSize, coreTraces) {
}

/*
 * If forwarderState = NO_FORWARDER, no cache available to forward block; serve
 * request from main memory instead.
 */
pair<int, string> MESIFRunner::getCopiesWithForwarder(int addr) {
    int numCopies = 0;
    string forwarderState = NO_FORWARDER;

    for (int cacheID = 0; cacheID < (int) caches.size(); cacheID++) {
        Cache& cache = caches[cacheID];

        if (cache.hasEntry(addr)) {
            numCopies += 1;
            string blockState = cache.getBlockState(addr);

            if (blockState == "M" || blockState == "E") {
                return make_pair(numCopies, blockState);
            } else if (blockState == "F") {
                // Only 'F' state cache forwards - 'S' state caches are silent
                forwarderState = blockState;
            }
        }
    }
 
    return make_pair(numCopies, forwarderState);
}

bool MESIFRunner::fetchAndAlloc(int cacheID, int addr,
        string newState, string forwarderState) {
    // Allocate a line from the requesting cache, evicting a block if needed
    Cache& cache = caches[cacheID];
    int blockNum = cache.getBlockNumber(addr);

    // Determine earliest time requested block (incl. all pending write-backs)
    // is available from memory
    int availableTime = getMemBlockAvailableTime(blockNum) + 100;
 
    // Pre-condition: find an available line (evict one if none available)
    CacheEntry evictedEntry = cache.evictEntry(addr);
    // If entry is valid, this is the victim block (cache set conflict)
    if (!evictedEntry.isInvalid()) {
        // Perform write-back to update mem if victim block in 'M' state; the
        // line will then be unavailable for 100 extra cycles pending write-back
        if (evictedEntry.getState() == "M") {
            int evictedAddr = cache.getHeadAddr(evictedEntry);
            cacheWriteBackMem(cacheID, evictedAddr);
        }
    }

    // If forwarding cache exists, assume served by faster of memory (including
    // pending write-backs) and the sole forwarding cache, as memory has fixed
    // 100 cycle penalty
    if (forwarderState != NO_FORWARDER) {
        // Time to transfer block fully from forwarding cache begins when cache
        // is done evicting 'M' victim block
        int cacheTransferTime = curTime + 2 * bus.getWordPerBlock();
        availableTime = min(availableTime, cacheTransferTime);
    }

    cache.allocEntry(addr, newState, curTime, availableTime);
    return true;
}

void MESIFRunner::invalidateO(int cacheID, int addr, bool needWriteBack) {
    // Invalidate all other copies in other caches
    for(int othCacheID = 0; othCacheID < (int) caches.size(); othCacheID++) {
        if (othCacheID == cacheID) continue;
        Cache& othCache = caches[othCacheID];

        if (othCache.hasEntry(addr)) {
            // If copy is dirty ('M' state) - only one process holds line
            if (needWriteBack && othCache.isAddrDirty(addr)) {
                cacheWriteBackMem(othCacheID, addr);
            }

            // Otherwise immediately invalidate that entry, whatever its state
            othCache.setBlockState(addr, "I");

            // Update stat 7
            bus.incInvalidateCount();
        }
    }
}

/*
 * No change to read hit behaviour in MESIF vs. MESI
 */
void MESIFRunner::simulateReadHit(int coreID, int addr) {
    Cache& cache = caches[coreID];
    cache.setBlockLastUsed(addr, curTime);
}

/*
 * Changes to write hit behaviour in MESIF vs. MESI: write hits are deferred
 * until cache servicing a read miss (BusRd) with a block C2C transfer is done
 */
void MESIFRunner::simulateWriteHit(int coreID, int addr) {
    int cacheID = coreID;
    Cache& cache = caches[cacheID];
    string blockState = cache.getBlockState(addr);

    cache.setBlockLastUsed(addr, curTime);
    cache.setBlockState(addr, "M");

    // Should have no other copies in 'M'/'E' state so this will only invalidate
    // other copies with no write-back needed
    invalidateO(cacheID, addr, false);

    // Mem does not hold an updated copy
    int blockNum = cache.getBlockNumber(addr);
    invalidBlock[blockNum] = INF;
}

/*
 * Enables FlushOpt (C2C transfer) of dirty blocks from M state (with queued
 * write-back) and clean blocks from E, F states.
 */
void MESIFRunner::simulateReadMiss(int coreID, int addr) {
    int numCaches = (int) caches.size();

    int numCopies;
    string forwarderState;
    tie(numCopies, forwarderState) = getCopiesWithForwarder(addr);

    // Perform state change in forwarding cache
    if (forwarderState != NO_FORWARDER) {
        for (int cacheID = 0; cacheID < numCaches; cacheID++) {
            Cache& cache = caches[cacheID];
 
            if (cache.hasEntry(addr)) {
                string blockState = cache.getBlockState(addr);

                if (blockState == "S") continue;
                assert(blockState == forwarderState);

                if (blockState == "M") {
                    // Forwarding cache is in 'M' state and holds a dirty copy
                    // Snooped response triggers flush in cache with 'M' state
                    cacheWriteBackMem(cacheID, addr);
                }

                // Forwarding cache has read-only copy, transitions to 'S'
                // Requesting cache takes its place as forwarder
                cache.setBlockState(addr, "S");
                break;
            }
        }
    }

    // If cache will hold the only read-only copy, transition to 'E' state (and
    // service from main memory)
    // Otherwise, cache will hold one of multiple read-only copies, transition
    // to 'F' state
    string addrState = (numCopies == 0) ? "E" : "F";
    fetchAndAlloc(coreID, addr, addrState, forwarderState);

    // Update stat 6 + 7
    bus.incTrafficBlock();
}

/*
 * Enables FlushOpt (C2C transfer) of dirty blocks from M state (with queued
 * write-back) and clean blocks from E, F states.
 */
void MESIFRunner::simulateWriteMiss(int coreID, int addr) {
    int numCopies;
    string forwarderState;
    tie(numCopies, forwarderState) = getCopiesWithForwarder(addr);

    // Invalidate with write-back if another cache a copy in 'M' state
    invalidateO(coreID, addr, true);

    // Transition to 'M' state
    string addrState = "M";
    fetchAndAlloc(coreID, addr, addrState, forwarderState);

    // Memory does not hold an updated copy
    int blockNum = caches[coreID].getBlockNumber(addr);
    invalidBlock[blockNum] = INF;

    // Update stat 6 + 7
    bus.incTrafficBlock();
}

void MESIFRunner::progressTime(int newTime) {
    for(auto &core : cores) {
        core.progress(newTime - curTime);
    }

    curTime = newTime;
    checkMem();
}

MESIFRunner::~MESIFRunner() {
}

