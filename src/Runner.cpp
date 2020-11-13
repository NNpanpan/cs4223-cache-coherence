#include <bits/stdc++.h>

#include "Runner.h"

using namespace std;

Runner::Runner(int cacheSize, int assoc, int blockSize,
        vector<vector<pair<int, int>>> coreTraces) : bus(blockSize) {
    curTime = 0;
    this->blockSize = blockSize;

    int numberOfCores = 4;
    for(int ID = 0; ID < numberOfCores; ID++) {
        caches.push_back(Cache(assoc, blockSize, cacheSize, ID));
        cores.push_back(Core(coreTraces[ID], ID));
    }
}

int Runner::getHeadAddr(int addr) {
    return (addr / blockSize) * blockSize;
}

int Runner::getMemBlockAvailableTime(int blockNum) {
    auto ite = invalidBlock.find(blockNum);
    if (ite == invalidBlock.end()) {
        return curTime;
    }

    assert(ite->second >= curTime);
    return ite->second;
}

void Runner::cacheWriteBackMem(int cacheID, int addr) {
    Cache& cache = caches[cacheID];
    int blockNum = cache.getBlockNumber(addr);

    // Memory holds stale copy of this block until flush completes
    assert(invalidBlock[blockNum] == INF);
    invalidBlock[blockNum] = curTime + 100;

    // Update stats 6 + 7
    bus.incWritebackCount();
    bus.incTrafficBlock();
}

void Runner::checkMem() {
    vector<int> unfreezeBlock;
    for(auto ite : invalidBlock) if (ite.second == curTime) {
        unfreezeBlock.push_back(ite.first);
    }

    for(auto block : unfreezeBlock) {
        invalidBlock.erase(block);
    }
}

// Helper functions

void Runner::printStat() {
    cout << fixed;

    int numCores = (int) cores.size();

    cout << "Stat 1: (Exec time)\n";
    int maxExecTime = -1;
    for(int coreID = 0; coreID < numCores; coreID++) {
        Core& core = cores[coreID];
        int coreExecTime = core.getExecCycles();
        maxExecTime = max(maxExecTime, coreExecTime);
        cout << "Core " << coreID << ": " << coreExecTime << " cycle(s)\n";
    }
    cout << "Total: " << maxExecTime << " cycle(s)\n";

    cout << "------------------------------------\n";

    cout << "Stat 2: (Compute time)\n";
    for(int coreID = 0; coreID < numCores; coreID++) {
        Core& core = cores[coreID];
        cout << "Core " << coreID << ": " << core.getCompCycles() << " cycle(s)\n";
    }

    cout << "------------------------------------\n";


    cout << "Stat 3: (Load/Store)\n";
    for(int coreID = 0; coreID < numCores; coreID++) {
        Core& core = cores[coreID];
        cout << "Core " << coreID << ": " << core.getLoadCount() << " load(s), " << core.getStoreCount() << " store(s)\n";
    }

    cout << "------------------------------------\n";

    cout << "Stat 4: (Idle time)\n";
    for(int coreID = 0; coreID < numCores; coreID++) {
        Core& core = cores[coreID];
        cout << "Core " << coreID << ": " << core.getIdleCycles() << " cycle(s)\n";
        // Sanity check
        assert(core.getIdleCycles() + core.getCompCycles() == core.getExecCycles());
    }

    cout << "------------------------------------\n";

    cout << "Stat 5: (Cache miss rate)\n";
    for(int coreID = 0; coreID < numCores; coreID++) {
        Core& core = cores[coreID];
        double ratio = (core.getCacheMissCount() + .0) / (core.getLoadCount() + core.getStoreCount());
        cout << setprecision(2) << "Core " << coreID << ": " << ratio * 100 << "%\n";
    }

    cout << "------------------------------------\n";

    cout << "Stat 6: (Bus traffic amount)\n";
    cout << bus.getTrafficData() << " byte(s)\n";

    cout << "------------------------------------\n";

    cout << "Stat 7: (Invalidation / Update / Write-back)\n";
    cout << "Invalidation: " << bus.getInvalidateCount() << " time(s)\n";
    cout << "Update: " << bus.getUpdateCount() << " time(s)\n";
    cout << "Write-back: " << bus.getWritebackCount() << " time(s)\n";

    cout << "------------------------------------\n";

    cout << "Stat 8: (Private access distribution)\n";
    int totalPriv = 0;
    int totalAcc = 0;
    cout << "Per core\n";
    for(int coreID = 0; coreID < numCores; coreID++) {
        Core& core = cores[coreID];
        int priv = core.getPrivateAccessCount();
        int tot = core.getLoadCount() + core.getStoreCount();
        double ratio = (priv + .0) / tot;
        cout << setprecision(2) << "Core " << coreID << ": " << priv << " / " << tot << " = " << ratio * 100 << "%\n";

        totalPriv += priv;
        totalAcc += tot;
    }

    double totalRatio = (totalPriv + .0) / totalAcc;
    cout << "Total : ";
    cout << setprecision(2) << totalPriv << " / " << totalAcc << " = " << totalRatio * 100 << "%\n";
}

bool Runner::isAllFinish() {
    for(auto &core: cores)
        if (!core.isFinish()) return false;
    return true;
}

// Checks if any core is released (not busy with compute or blocked)
bool Runner::checkReleaseCore() {
    bool exist = false;
    for(int coreID = 0; coreID < (int) cores.size(); coreID++) {
        Core& core = cores[coreID];
        if (core.isFinish()) continue;  // Freeze finished core
        core.refresh(curTime);

        if (core.isFree()) {
            exist = true;
        }
    }

    vector<int> doneBlocks;
    for (auto elem : activeBlocks) {
        int coreID = elem.second;
        Core& core = cores[coreID];

        // Pending request on data bus (block fetch/C2C transfer) is done
        if (core.isFree() || core.isFinish()) {
            doneBlocks.push_back(elem.first);
        }
    }

    for (auto block : doneBlocks) {
        activeBlocks.erase(block);
    }

    return exist;
}

bool sortCores(const pair<int, pair<int, int>> &a, const pair<int, pair<int, int>> &b) {
    // First element is core's nextFree
    // Second element is core's lastCacheReq, third is core ID
    if (a.first == b.first) {
        return a.second.first < b.second.first;
    }

    return a.first < b.first;
}

bool Runner::checkCoreReq() {
    bool exist = false;
    bool serveCacheReq = false;

    // vector<pair<int, int>> coreOrder;
    vector<pair<int, pair<int, int>>> coreOrder;
    for(int coreID = 0; coreID < (int) cores.size(); coreID++) {
        Core& core = cores[coreID];
        if (core.isFinish()) continue;  // Freeze finished core
        if (!core.isFree()) continue;
        // coreOrder.push_back(make_pair(core.getNextFree(), coreID));
        coreOrder.push_back(make_pair(core.getNextFree(),
            make_pair(core.getLastCacheReq(), coreID)));
    }

    sort(coreOrder.begin(), coreOrder.end(), sortCores);

    for(auto i : coreOrder) {           // Ensure priority
        int coreID = i.second.second;
        Core& core = cores[coreID];

        assert(!core.isFinish());
        assert(core.isFree());

        pair<int, int> trace = core.peekTrace();

        int traceType = trace.first;
        int addr = trace.second;

        if (traceType == 0 || traceType == 1) { // Load/store instruction
            Cache& cache = caches[coreID];
            // Check if cache holds block containing this addr
            if (cache.hasEntry(addr)) {
                // Core always able to proceed if it has the cache line
                exist = true;
                // Read hit
                if (traceType == 0) {
                    core.popTrace();
                    simulateReadHit(cache.getID(), addr);
                }
                // Write hit
                if (traceType == 1) {
                    // Can only write if the cache line is not in transaction
                    if (activeBlocks.find(getHeadAddr(addr)) == activeBlocks.end()) {
                        earlyRet = false;
                        simulateWriteHit(cache.getID(), addr);
                        if (earlyRet) {
                            core.incIdleCycles(1);
                            continue;
                        } else {
                            core.popTrace();
                        }
                    } else {
                        core.incIdleCycles(1);
                        continue;
                    }
                }
            } else {
                // Can delay cache request and update of stats
                // Since for cache miss, snooping won't lead to hit
                if (serveCacheReq ||
                    activeBlocks.find(getHeadAddr(addr)) != activeBlocks.end()) {
                    // Stall core if bus already serving a cache request or if
                    // the requested block is already being requested
                    core.incIdleCycles(1);
                    continue;
                }

                serveCacheReq = true;
                exist = true;
                core.popTrace();
                // Cache miss -> update stat 5
                core.incCacheMissCount();
                activeBlocks[getHeadAddr(addr)] = coreID;
                core.setLastCacheReq(curTime);

                // Read miss
                if (traceType == 0) {
                    simulateReadMiss(cache.getID(), addr);
                }
                // Write miss
                if (traceType == 1) {
                    simulateWriteMiss(cache.getID(), addr);
                }
            }

            // Here: cache request is served by bus
            // If private access (in E or M state) -> update stat 8
            if (cache.isAddrPrivate(addr)) {
                core.incPrivateAccessCount();
            }

            // Load/store -> update stat 3
            if (traceType == 0) {
                core.incLoadCount();
            } else {
                core.incStoreCount();
            }

            int nextFree = max(curTime, cache.getAddrUsableTime(addr)) + 1;
            core.incIdleCycles(nextFree - curTime);
            core.setBusy(nextFree);
        }

        if (traceType == 2) {       // Compute instruction
            exist = true;
            core.popTrace();        // Pop core trace

            int computeTime = trace.second;
            // Mark core as busy for (computeTime) duration
            core.setBusy(curTime + computeTime);
            // Update stat 4
            core.incCompCycles(computeTime);
        }
    }

    return exist;
}

void Runner::simulate() {
    while (!isAllFinish()) {
        if (curTime % 1000000 == 0) {
            cout << "Cycles " << curTime << endl;
            for(int coreID = 0; coreID < (int) caches.size(); coreID++) {
                cout << cores[coreID].traceQ.size() << " ";
            }
            cout << endl;
        }

        // Allow cores to retrieve from memory if cache block is available
        checkReleaseCore();
        checkCoreReq();

        // This is slow but maybe okay, go for correctness first
        progressTime(curTime + 1);
    }
    printStat();
}

Runner::~Runner() {
}

