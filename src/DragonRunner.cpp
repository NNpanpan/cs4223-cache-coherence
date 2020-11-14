#include "DragonRunner.h"

DragonRunner::DragonRunner(int cacheSize, int assoc, int blockSize,
    vector<vector<pair<int, int>>> coreTraces)
    : Runner(cacheSize, assoc, blockSize, coreTraces) {
}

int DragonRunner::findCacheSourceAvailableTime(int cacheID, int addr) {
    // Assuming cache controller can determine data source with lowest latency
    // between main memory and another cache
    int availableTimeFromOth = INF;

    for(int othCacheID = 0; othCacheID < (int) caches.size(); othCacheID++) {
        if (othCacheID == cacheID) continue;
        Cache& othCache = caches[othCacheID];
        if (othCache.hasEntry(addr)) {
            availableTimeFromOth = min(availableTimeFromOth,
                max(othCache.getAddrUsableTime(addr), curTime));
        }
    }
    return availableTimeFromOth;
}

int DragonRunner::findMemSourceAvailableTime(int cacheID, int addr) {
    // Assuming cache controller can determine data source with lowest latency
    // between main memory and another cache
    Cache& cache = caches[cacheID];

    int blockNum = cache.getBlockNumber(addr);

    int availableTimeFromMem = getMemBlockAvailableTime(blockNum);
    return availableTimeFromMem;
}

int DragonRunner::findSourceAvailableTime(int cacheID, int addr) {
    // Assuming cache controller can determine data source with lowest latency
    // between main memory and another cache
    int availableTimeFromMem = findMemSourceAvailableTime(cacheID, addr);
    int availableTimeFromCache = findCacheSourceAvailableTime(cacheID, addr);
    return min(availableTimeFromMem + 100,
        availableTimeFromCache + 2 * bus.getWordPerBlock());
}

int DragonRunner::countOthCacheHold(int cacheID, int addr) {
    int countHold = 0;
    for(int othCacheID = 0; othCacheID < (int) caches.size(); othCacheID++) {
        if (othCacheID == cacheID) continue;
        Cache& othCache = caches[othCacheID];
        if (othCache.hasEntry(addr)) {
            countHold++;
        }
    }
    return countHold;
}

void DragonRunner::cacheReceiveW(int cacheID, int addr, int sendCycle) {
    Cache& cache = caches[cacheID];
    assert(cache.hasEntry(addr));

    // 2 cycles to transmit updated word (after which block marked as valid)
    cache.setBlockValidFrom(addr, sendCycle + 2);

    // Update stat 6
    bus.incTrafficWord();
}

void DragonRunner::cacheReceiveB(int cacheID, int addr, string state) {
    // Passive receipt of block
    Cache& cache = caches[cacheID];
    assert(!cache.hasEntry(addr));

    /*
    int availableTime = findSourceAvailableTime(cacheID, addr);
    assert(availableTime < INF);
    */

    int cacheAvailableTime = max(findCacheSourceAvailableTime(cacheID, addr), curTime);
    int memAvailableTime = max(findMemSourceAvailableTime(cacheID, addr), curTime);
    /*
    int availableTime = min(memAvailableTime + 100,
        cacheAvailableTime + 2 * bus.getWordPerBlock());
    */

    // Serve from memory if no cache holds a copy, else perform C2C transfer
    int availableTime = cacheAvailableTime == INF 
        ? memAvailableTime + 100 
        : cacheAvailableTime + 2 * bus.getWordPerBlock();

    // Pre-condition: find an available line (evict one if none available)
    CacheEntry evictedEntry = cache.evictEntry(addr);
    // If entry is valid, this is the victim block (cache set conflict)
    if (!evictedEntry.isInvalid()) {
        int evictedAddr = cache.getHeadAddr(evictedEntry);
        int evictedBlockNum = cache.getBlockNumber(evictedAddr);
        bool needRewrite = (getMemBlockAvailableTime(evictedBlockNum) == INF 
            && countOthCacheHold(cacheID, evictedAddr) == 0);

        // Perform write-back to update mem if memory holds a stale copy and
        // the evicted line is the last copy amongst all caches
        if (needRewrite) {
            cacheWriteBackMem(cacheID, evictedAddr);

            /*
             * No additional latency to fill line after eviction with write-back
             * (assuming use of line fill buffer and write-back buffer)
             *
            int newMemAvailableTime = max(curTime + 100, memAvailableTime);
            int newCacheAvailableTime = max(curTime + 100, cacheAvailableTime);
            availableTime = min(newMemAvailableTime + 100,
                newCacheAvailableTime + 2 * bus.getWordPerBlock());
            availableTime = newCacheAvailableTime == INF 
                ? newMemAvailableTime + 100 
                : newCacheAvailableTime + 2 * bus.getWordPerBlock();
            availableTime = max(availableTime, curTime + 100);
             */
        }
    }

    cache.allocEntry(addr, state, curTime, availableTime);

    // Update stat 6
    bus.incTrafficBlock();
}

void DragonRunner::broadcastWOthCache(int cacheID, int addr, int sendCycle) {
    // cout << "(to) broadcast word from cache " << cacheID << " at time "
    //     << sendCycle << " and done at " << sendCycle+2 << endl;

    // Broadcast a word to other caches (update)
    int countHold = countOthCacheHold(cacheID, addr);
    int headAddr = getHeadAddr(addr);
    assert(countHold > 0);
    broadcastingBlocks[headAddr] = sendCycle + 2;

    for(int othCacheID = 0; othCacheID < (int) caches.size(); othCacheID++) {
        if (othCacheID == cacheID) continue;
        Cache& othCache = caches[othCacheID];
        if (othCache.hasEntry(addr)) {
            cacheReceiveW(othCacheID, addr, sendCycle);
            othCache.setBlockState(addr, "Sc");

            // Update stat 7
            bus.incUpdateCount();
        }
    }

    // cout << "(to) broadcast word from cache " << cacheID << " at time "
    //     << sendCycle << " and done at " << sendCycle+2 << endl;
}

void DragonRunner::simulateReadHit(int coreID, int addr) {
    Cache& cache = caches[coreID];
    string state = cache.getBlockState(addr);
    
    // Do nothing; set last used and done
    cache.setBlockLastUsed(addr, curTime);
}

void DragonRunner::simulateWriteHit(int coreID, int addr) {
    if (broadcastingBlocks.find(getHeadAddr(addr)) != broadcastingBlocks.end()) {
        earlyRet = true;
        return;
    }

    int cacheID = coreID;
    Cache& cache = caches[coreID];
    string state = cache.getBlockState(addr);
    cache.setBlockLastUsed(addr, curTime);

    if (state == "M") {
        // Do nothing
    }

    if (state == "Sc" || state == "Sm") {
        // Check if cache should transition to 'M' or 'Sm'
        int countHold = countOthCacheHold(coreID, addr);

        string addrState = (countHold == 0) ? "M" : "Sm";
        if (addrState == "Sm") {
            // Broadcast the modified word to other caches
            broadcastWOthCache(cacheID, addr, curTime);
        } else {
            /// do nothing
        }
        cache.setBlockState(addr, addrState);
    }

    if (state == "E") {
        // Transition to 'M'
        cache.setBlockState(addr, "M");
    }

    // Mem does not hold an updated copy
    int blockNum = cache.getBlockNumber(addr);
    invalidBlock[blockNum] = INF;
}

void DragonRunner::simulateReadMiss(int coreID, int addr) {
    int countHold = countOthCacheHold(coreID, addr);
    string state = (countHold == 0) ? "E" : "Sc";
    cacheReceiveB(coreID, addr, state);
}

void DragonRunner::simulateWriteMiss(int coreID, int addr) {
    int cacheID = coreID;
    Cache& cache = caches[coreID];
    int countHold = countOthCacheHold(coreID, addr);

    string state = (countHold == 0) ? "M" : "Sm";

    if (state == "M") {
        cacheReceiveB(coreID, addr, "M");
    } else {
        // Receive a block transfer, then broadcast the updated word to others
        cacheReceiveB(coreID, addr, "Sm");

        int sendTime = cache.getAddrUsableTime(addr);
        broadcastWOthCache(cacheID, addr, sendTime);
    }

    // Memory does not hold latest copy of this block
    int blockNum = cache.getBlockNumber(addr);
    invalidBlock[blockNum] = INF;
}

void DragonRunner::progressTime(int newTime) {
    for(auto &core : cores) {
        core.progress(newTime - curTime);
    }

    curTime = newTime;
    checkMem();

    vector<int> doneBlocks;
    for (auto elem : broadcastingBlocks) {
        int expiry = elem.second;

        if (curTime >= expiry) {
            // A block with a word broadcasted via BusUpd that completed
            doneBlocks.push_back(elem.first);
        }
    }

    // Resume bus transactions for blocks whose BusUpd completed
    for (auto block : doneBlocks) {
        // cout << "Cycle " << curTime << " done broadcast word of block "
        //      << block << endl;
        broadcastingBlocks.erase(block);
    }
}

DragonRunner::~DragonRunner() {
}

