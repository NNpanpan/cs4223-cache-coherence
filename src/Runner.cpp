#include "Runner.h"

Runner::Runner(int cacheSize, int assoc, int blockSize,
        vector<vector<pair<int, int>>> coreTraces) : bus(blockSize) {

    int numberOfCores = 4;
    for(int ID = 0; ID < numberOfCores; ID++)
        caches.push_back(Cache(assoc, blockSize, cacheSize, ID));
    for(int ID = 0; ID < numberOfCores; ID++)
        cores.push_back(Core(coreTraces[ID], ID));
    curTime = 0;
}


/// helper

void Runner::printStat() {
    /// TBD : should be simple enough
}

bool Runner::isAllFinish() {
    for(auto &core: cores)
        if (!core.isFinish()) return false;
    for(auto &cache: caches)
        if (!cache.isFree()) return false;
    if (!bus.isFree()) return false;
    return true;
}


void Runner::progressTime(int newTime) {
    int span = newTime - curTime;
    for(auto &core : cores) { ///reference
        if (core.isFinish()) {
            continue;
        }
        assert(!core.isFree()); /// should not be free
        core.progress(span);
    }
    curTime = newTime;
}

void Runner::addCacheReq(CacheReq req) {
    pendingReq.insert(req);
}

bool Runner::checkReleaseCache() {
    bool exist = false;
    for(auto& cache: caches) {
        cache.refresh(curTime);
        if (cache.isFree()) exist = true;
    }
    return exist;
}

bool Runner::checkReleaseBus() {
    bus.refresh(curTime);
    return bus.isFree();
}

bool Runner::checkReleaseCore() { /// if any core got release
    bool exist = false;
    for(int coreID = 0; coreID < cores.size(); coreID++) {
        Core& core = cores[coreID];
        if (core.isFinish()) continue;

        core.refresh(curTime);
        if (core.isFree()) { /// only able to change if is busy
            exist = true;
            core.popTrace(); /// pop compute request
        }
        if (core.isBusyWait()) { /// then must be waiting for some address
            Cache& cache = caches[coreID]; /// correspond cache

            /// check if cache hold the address this core need and is free
            pair<int, int> trace = core.peekTrace();
            int addr = trace.second;
            if (cache.isFree() && cache.hasEntry(addr)) {
                exist = true;
                /// private access -> update stat 8
                if (cache.isAddrPrivate(addr)) {
                    core.incPrivateAccessCount();
                }
                /// state of entry is already correct, pop trace and go back to free
                core.popTrace();
                core.setFree();
            }
        }
    }
    return exist;
}
bool Runner::checkCoreReq() {
    bool exist = false;
    for(int coreID = 0; coreID < cores.size(); coreID++) { /// ensure priority
        Core& core = cores[coreID];
        if (!core.isFree()) continue;

        pair<int, int> trace = core.peekTrace();
        int traceType = trace.first;
        if (traceType == 0 || traceType == 1) { /// load/store instruction
            Cache& cache = caches[coreID];
            if (!cache.isFree()) continue;

            exist = true;
            /// load/store -> update stat 3
            if (traceType == 0) {
                core.incLoadCount();
            } else {
                core.incStoreCount();
            }

            int addr = trace.second;
            core.setBusyWait(); /// core go into busy waiting

            ///check if the cache hold this addr
            if (cache.hasEntry(addr)) {
                /// read hit
                if (traceType == 0) {
                    simulateReadHit(cache.getID(), addr);
                }
                /// write hit
                if (traceType == 1) {
                    simulateWriteHit(cache.getID(), addr);
                }
            } else { ///cache miss
                /// cache miss -> update stat 5
                core.incCacheMissCount();

                /// read miss
                if (traceType == 0) {
                    simulateReadMiss(cache.getID(), addr);
                }

                /// write miss
                if (traceType == 1) {
                    simulateWriteMiss(cache.getID(), addr);
                }
            }
        }
        if (traceType == 2) { ///compute instruction
            exist = true;

            int computeTime = trace.second;
            core.setBusy(curTime + computeTime); /// set busy for computeTime duration
        }
    }
    return exist;
}

int Runner::findNextEvent() { /// TBD: recheck correctness
    int event = 2e9 + 10;
    for(auto &core: cores) if (!core.isFinish())
        event = min(event, core.getNextFree());
    for(auto &cache: caches) if (!cache.isFree())
        event = min(event, cache.getNextFree());

    if (!bus.isFree())
        event = min(event, bus.getNextFree());
    return true;
}
void Runner::simulate() {
    while (!isAllFinish()) {
        // printDebug();
        /// release priority
        checkReleaseBus();
        checkReleaseCache();

        /// this will also allow core to retrieve memory if cache is free
        checkReleaseCore();

        /// first try to satisfy any request from cache
        bool haveCacheReq = false;
        for(auto req : pendingReq) {
            Cache& reqCache = caches[req.getCacheID()];
            /// both bus and cache should be free to exec
            if (!bus.isBusy() && !reqCache.isBusy()) {
                haveCacheReq = true;
                pendingReq.erase(req);
                execCacheReq(req); /// this call respective protocol
                break;
            }
        }
        if (haveCacheReq) {
            continue;
        }

        /// proceed with request from free core
        bool isCoreReqMaked = checkCoreReq();

        /// if there are request, then this cycle is not fully simulated yet
        if (isCoreReqMaked) {
            continue;
        }

        /// else need to progress to next event when (something which is currently not free) is free
        int newTime = findNextEvent();
        progressTime(newTime);
    }
}


