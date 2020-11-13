#include "DragonRunner.h"
// #include <bits/stdc++.h>
// using namespace std;

DragonRunner::DragonRunner(int cacheSize, int assoc, int blockSize,
    vector<vector<pair<int, int>>> coreTraces)
    : Runner(cacheSize, assoc, blockSize, coreTraces) {
}

int DragonRunner::findCacheSourceAvailableTime(int cacheID, int addr) {
    /// assume there is someway to figure out which is better
    Cache& cache = caches[cacheID];
    int availableTimeFromOth = INF;

    for(int othCacheID = 0; othCacheID < caches.size(); othCacheID++) {
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
    /// assume there is someway to figure out which is better
    Cache& cache = caches[cacheID];

    int blockNum = cache.getBlockNumber(addr);

    int availableTimeFromMem = getMemBlockAvailableTime(blockNum);
    return availableTimeFromMem;
}

int DragonRunner::findSourceAvailableTime(int cacheID, int addr) {
    /// assume there is someway to figure out which is better
    Cache& cache = caches[cacheID];

    int blockNum = cache.getBlockNumber(addr);
    int availableTimeFromMem = findMemSourceAvailableTime(cacheID, addr);
    int availableTimeFromCache = findCacheSourceAvailableTime(cacheID, addr);
    return min(availableTimeFromMem + 100,
        availableTimeFromCache + 2 * bus.getWordPerBlock());
}

int DragonRunner::countOthCacheHold(int cacheID, int addr) {
    int countHold = 0;
    for(int othCacheID = 0; othCacheID < caches.size(); othCacheID++) {
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

    /// 2 cycles
    cache.setBlockValidFrom(addr, sendCycle + 2);

    /// update stat 6
    bus.incTrafficWord();
}

void DragonRunner::cacheReceiveB(int cacheID, int addr, string state) {
    /// passive receive
    Cache& cache = caches[cacheID];
    assert(!cache.hasEntry(addr));

    /*
    int availableTime = findSourceAvailableTime(cacheID, addr);
    assert(availableTime < INF);
    */

    int cacheAvailableTime = max(findCacheSourceAvailableTime(cacheID, addr), curTime);
    int memAvailableTime = max(findMemSourceAvailableTime(cacheID, addr), curTime);
    // int availableTime = min(memAvailableTime + 100, cacheAvailableTime + 2 * bus.getWordPerBlock());
    int availableTime = cacheAvailableTime == INF 
        ? memAvailableTime + 100 
        : cacheAvailableTime + 2 * bus.getWordPerBlock();

    /// assuming evict before alloc
    CacheEntry evictedEntry = cache.evictEntry(addr);
    /// if that entry is valid, mean cache set conflict
    if (!evictedEntry.isInvalid()) {
        /// need to rewrite if mem does not hold and is last copy in caches
        int evictedAddr = cache.getHeadAddr(evictedEntry);
        int evictedBlockNum = cache.getBlockNumber(evictedAddr);
        bool needRewrite = (getMemBlockAvailableTime(evictedBlockNum) == INF 
            && countOthCacheHold(cacheID, evictedAddr) == 0); 
        if (needRewrite) {
            cacheWriteBackMem(cacheID, evictedAddr);

            int newMemAvailableTime = max(curTime + 100, memAvailableTime);
            int newCacheAvailableTime = max(curTime + 100, cacheAvailableTime);
            // availableTime = min(newMemAvailableTime + 100, newCacheAvailableTime + 2 * bus.getWordPerBlock()); /// 100 cycles first to evict this addr
            availableTime = newCacheAvailableTime == INF 
                ? newMemAvailableTime + 100 
                : newCacheAvailableTime + 2 * bus.getWordPerBlock();

            /*
            availableTime = max(availableTime, curTime + 100);
            */
        }
    }

    cache.allocEntry(addr, state, curTime, availableTime);

    /// update stat 6
    bus.incTrafficBlock();
}

void DragonRunner::broadcastWOthCache(int cacheID, int addr, int sendCycle) {
    // cout << "(to) broadcast word from cache " << cacheID << " at time " << sendCycle << " and done at " << sendCycle+2 << endl;
    /// word broadcast
    int countHold = countOthCacheHold(cacheID, addr);
    int headAddr = getHeadAddr(addr);
    assert(countHold > 0);
    broadcastingBlocks[headAddr] = sendCycle + 2;

    for(int othCacheID = 0; othCacheID < caches.size(); othCacheID++) {
        if (othCacheID == cacheID) continue;
        Cache& othCache = caches[othCacheID];
        if (othCache.hasEntry(addr)) {
            cacheReceiveW(othCacheID, addr, sendCycle);
            othCache.setBlockState(addr, "Sc");

            /// update stat 7
            bus.incUpdateCount();
        }
    }
    // cout << "(to) broadcast word from cache " << cacheID << " at time " << sendCycle << " and done at " << sendCycle+2 << endl;
}

void DragonRunner::simulateReadHit(int coreID, int addr) {
    Cache& cache = caches[coreID];
    string state = cache.getBlockState(addr);
    
    /// do nothing, set last use and done
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
        /// do nothing
    }

    if (state == "Sc" || state == "Sm") {
        /// check if cache should go to 'M' or 'Sm'
        int countHold = countOthCacheHold(coreID, addr);

        string addrState = (countHold == 0) ? "M" : "Sm";
        if (addrState == "Sm") {
            /// broadcast to other caches a word
            broadcastWOthCache(cacheID, addr, curTime);
        } else {
            /// do nothing
        }
        cache.setBlockState(addr, addrState);
    }

    if (state == "E") {
        /// go to M
        cache.setBlockState(addr, "M");
    }

    /// memory does not hold this block
    int blockNum = cache.getBlockNumber(addr);
    invalidBlock[blockNum] = INF;
}

void DragonRunner::simulateReadMiss(int coreID, int addr) {
    Cache& cache = caches[coreID];
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
        /// need to receive a block broadcast and then broadcast a word to other cache
        cacheReceiveB(coreID, addr, "Sm");

        int sendTime = cache.getAddrUsableTime(addr);
        broadcastWOthCache(cacheID, addr, sendTime);
    }

    /// memory does not hold this block
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
            // A block with a word broadcast
            doneBlocks.push_back(elem.first);
        }
    }

    for (auto block : doneBlocks) {
        // cout << "Cycle " << curTime << " done broadcast word of block " << block << endl;
        broadcastingBlocks.erase(block);
    }
}

DragonRunner::~DragonRunner() {
}
