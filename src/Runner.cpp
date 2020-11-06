#include <bits/stdc++.h>

using namespace std;

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
    cout << fixed;

    cout << "Stat 1: (Exec time)\n";
    int maxExecTime = -1;
    for(int coreID = 0; coreID < cores.size(); coreID++) {
        Core& core = cores[coreID];
        int coreExecTime = core.getExecCycles();
        maxExecTime = max(maxExecTime, coreExecTime);
        cout << "Core " << coreID << ": " << coreExecTime << " cycle(s)\n";
    }
    cout << "Total: " << maxExecTime << " cycle(s)\n";

    cout << "------------------------------------\n";

    cout << "Stat 2: (Compute time)\n";
    for(int coreID = 0; coreID < cores.size(); coreID++) {
        Core& core = cores[coreID];
        cout << "Core " << coreID << ": " << core.getExecCycles() << " cycle(s)\n";
    }

    cout << "------------------------------------\n";


    cout << "Stat 3: (Load/Store)\n";
    for(int coreID = 0; coreID < cores.size(); coreID++) {
        Core& core = cores[coreID];
        cout << "Core " << coreID << ": " << core.getLoadCount() << " load(s), " << core.getStoreCount() << " store(s)\n";
    }

    cout << "------------------------------------\n";

    cout << "Stat 4: (Idle time)\n";
    for(int coreID = 0; coreID < cores.size(); coreID++) {
        Core& core = cores[coreID];
        cout << "Core " << coreID << ": " << core.getIdleCycles() << " cycle(s)\n";
        /// sanity check 
        assert(core.getIdleCycles() + core.getCompCycles() == core.getExecCycles());
    }

    cout << "------------------------------------\n";

    cout << "Stat 5: (Cache miss rate)\n";
    for(int coreID = 0; coreID < cores.size(); coreID++) {
        Core& core = cores[coreID];
        double ratio = (core.getCacheMissCount() + .0) / (core.getLoadCount() + core.getStoreCount());
        cout << setprecision(2) << "Core " << coreID << ": " << ratio * 100 << "%\n";
    }

    cout << "------------------------------------\n";

    cout << "Stat 6: (Bus traffic size)\n";
    cout << bus.getTrafficData() << " byte(s)\n";

    cout << "------------------------------------\n";

    cout << "Stat 7: (Invalidation / Update)\n";
    cout << "Invalidation: " << bus.getInvalidateCount() << " time(s)\n";
    cout << "Update: " << bus.getUpdateCount() << " time(s)\n";

    cout << "------------------------------------\n";
    
    cout << "Stat 8: (Private access distribution)\n";
    int totalPriv = 0;
    int totalAcc = 0;
    cout << "Per core\n";
    for(int coreID = 0; coreID < cores.size(); coreID++) {
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


bool Runner::checkReleaseCore() { /// if any core got release
    bool exist = false;
    for(int coreID = 0; coreID < cores.size(); coreID++) {
        Core& core = cores[coreID];
        if (core.isFinish()) continue; /// freeze finish core
        core.refresh(curTime);
        
        if (core.isFree()) {
            exist = true;
        }
    }
    return exist;
}
bool Runner::checkCoreReq() {
    bool exist = false;
    bool serveCacheReq = false;

    vector<pair<int, int>> coreOrder;
    for(int coreID = 0; coreID < cores.size(); coreID++) { 
        Core& core = cores[coreID];
        if (core.isFinish()) continue; ///freeze finished core
        if (!core.isFree()) continue;
        coreOrder.push_back(make_pair(core.getNextFree(), coreID));
    }
    sort(coreOrder.begin(), coreOrder.end());

    for(auto i : coreOrder) { /// ensure priority
        int coreID = i.second;
        Core& core = cores[coreID];
        assert(!core.isFinish()); 

        assert(core.isFree());

        pair<int, int> trace = core.peekTrace();

        int traceType = trace.first;
        int addr = trace.second;

        if (traceType == 0 || traceType == 1) { /// load/store instruction
            Cache& cache = caches[coreID];
            ///check if the cache hold this addr
            if (cache.hasEntry(addr)) {
                /// no cache request generated
                exist = true;
                core.popTrace();
                /// read hit
                if (traceType == 0) {
                    simulateReadHit(cache.getID(), addr);
                }
                /// write hit
                if (traceType == 1) {
                    simulateWriteHit(cache.getID(), addr);
                }
            } else {
                /// can delay cache req and stat upd
                /// since if cache miss, then can not change to hit due to snooping
                if (serveCacheReq) {
                    core.incIdleCycles(1);
                    continue;
                }
                serveCacheReq = true;
                exist = true;
                core.popTrace();
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
            /// only reach here if the req is served
            /// if private access -> update stat 8
            if (cache.isAddrPrivate(addr)) {
                core.incPrivateAccessCount();
            }

            /// load/store -> update stat 3
            if (traceType == 0) {
                core.incLoadCount();
            } else {
                core.incStoreCount();
            }
            int nextFree = max(curTime, cache.getAddrUsableTime(addr)) + 1;
            core.incIdleCycles(nextFree - curTime);
            core.setBusy(nextFree);

        }
        if (traceType == 2) { ///compute instruction
            exist = true;
            core.popTrace(); /// pop core trace 

            int computeTime = trace.second;
            core.setBusy(curTime + computeTime); /// set busy for computeTime duration
            /// update stat 4 
            core.incCompCycles(computeTime);
        }
        //cout << "Core " << coreID << " free at " << core.getNextFree() << endl;
    }
    return exist;
}

void Runner::simulate() {
    while (!isAllFinish()) {
        if (curTime % 1000000 == 0) {
            cout << "Cycles " << curTime << endl;
            for(int coreID = 0; coreID < caches.size(); coreID++) {
                cout << cores[coreID].traceQ.size() << " ";
            }
            cout << endl;
        }
        /// allow core to retrieve memory if cache block is available
        checkReleaseCore();
        checkCoreReq();
        /// this is slow but maybe okay, go for correctness first
        progressTime(curTime + 1);
    }
    printStat();
}

Runner::~Runner() {
}
