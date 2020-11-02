#include "MESIRunner.h"

MESIRunner::MESIRunner(int cacheSize, int assoc, int blockSize,
    vector<vector<pair<int, int>>> coreTraces)
    : Runner(cacheSize, assoc, blockSize, coreTraces) {
}

bool MESIRunner::cacheAllocAddr(int cacheID, int addr, string addrState) { /// TBD: maybe recheck
    Cache& cache = caches[cacheID];
    CacheEntry evictedEntry = cache.evictEntry(addr);
    /// if that entry is valid, mean cache set conflict

    assert(bus.isFree());
    assert(cache.isFree());

    if (!evictedEntry.isInvalid()) {
        /// need to rewrite if in "M" state
        if (evictedEntry.getState() == "M") {
            /// TBD : side effect, careful or find someway
            int evictedAddr = cache.getHeadAddr(evictedEntry);
            addCacheReq(CacheReq(cacheID, evictedAddr, curTime, "wr"));
            return false;
        }
    }
    cache.allocEntry(addr, addrState, curTime);
    return true;
}

void MESIRunner::invalidateO(int cacheID, int addr) {
    /// invalidate all other caches
    for(int othCacheID = 0; othCacheID < caches.size(); othCacheID++) {
        if (othCacheID == cacheID) continue;
        Cache& othCache = caches[othCacheID];
        if (othCache.hasEntry(addr)) {
            /// immediately invalidate that entry, regardless of its state
            othCache.setBlockState(addr, "I");
            /// update stat 7
            bus.incInvalidateCount();
        }
    }
}

void MESIRunner::execCacheReq(CacheReq req) {
    string reqType = req.getReqType();
    int addr = req.getAddr();
    int cacheID = req.getCacheID();
    Cache &cache = caches[cacheID];
    Core &core = cores[cacheID];

    assert(bus.isFree());
    assert(cache.isFree());

    if (reqType == "rd") { /// read
        /// check if some cache has 'M'
        for(int othCacheID = 0; othCacheID < caches.size(); othCacheID++) {
            if (othCacheID == cacheID) continue;
            Cache& othCache = caches[othCacheID];
            if (othCache.hasEntry(addr) && othCache.isAddrPrivate(addr)) {
                /// that cache need to execute write
                addCacheReq(CacheReq(othCacheID, addr, curTime, "wr"));
                /// not fulfill yet, push request back to pending, set busy wait
                addCacheReq(req);
                cache.setBusyWait();
                return;
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
        if (!cacheAllocAddr(cacheID, addr, addrState)) {
            /// not fulfill yet
            addCacheReq(req);
            cache.setBusyWait();
        } else {
            /// both bus and cache busy for 100 cycles
            bus.setBusy(curTime + 100);
            cache.setBusy(curTime + 100);
        }
    }

    if (reqType == "rdX") {
        invalidateO(cacheID, addr);
        /// go to 'M' state
        string addrState = "M";
        if (!cacheAllocAddr(cacheID, addr, addrState)) {
            /// not fulfill yet
            addCacheReq(req);
            cache.setBusyWait();
        } else {
            bus.setBusy(curTime + 100);
            cache.setBusy(curTime + 100);
        }
    }
    if (reqType == "wr") {
        /// this cache should be in "M" and all other cache is in "I"
        assert(cache.isAddrPrivate(addr));

        cache.setBlockState(addr, "S");

        cache.setBusy(curTime + 100);
        bus.setBusy(curTime + 100);

        /// update stat 6 + 7
        bus.incUpdateCount();
        bus.incTrafficBlock();
    }
}
void MESIRunner::simulateReadHit(int coreID, int addr) {
    Cache& cache = caches[coreID];
    cache.setBlockLastUsed(addr, curTime);
    cache.setBusy(curTime + 1);
}
void MESIRunner::simulateWriteHit(int coreID, int addr) {
    int cacheID = coreID;
    Cache& cache = caches[cacheID];
    string blockState = cache.getBlockState(addr);

    cache.setBlockLastUsed(addr, curTime);
    cache.setBlockState(addr, "M");
    if (blockState == "S") {
        invalidateO(cacheID, addr);
    }
    cache.setBusy(curTime + 1);
    bus.setFree();
}
void MESIRunner::simulateReadMiss(int coreID, int addr) {
    addCacheReq(CacheReq(coreID, addr, curTime, "rd"));
}
void MESIRunner::simulateWriteMiss(int coreID, int addr) {
    addCacheReq(CacheReq(coreID, addr, curTime, "rdX"));
}


