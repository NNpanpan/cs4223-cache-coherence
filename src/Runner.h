#ifndef RUNNER_H
#define RUNNER_H

#include "Core.h"
#include "Bus.h"
#include "Cache.h"

using namespace std;

class Runner {
protected:
    vector<Core> cores;
    vector<Cache> caches;
    Bus bus;

    int curTime;

    void printStat();
    bool isAllFinish();

    bool checkReleaseCore();
    bool checkCoreReq();

    void printDebug();



public:
    Runner(int cacheSize, int assoc, int blockSize,
        vector<vector<pair<int, int>>> coreTraces);

    void simulate();
    //virtual bool execCacheReq(CacheReq req) = 0;

    /// note: these function use curTime, so have not count 1 cycle for cache check yet
    virtual void simulateReadHit(int coreID, int addr) = 0;
    virtual void simulateWriteHit(int coreID, int addr) = 0;
    virtual void simulateReadMiss(int coreID, int addr) = 0;
    virtual void simulateWriteMiss(int coreID, int addr) = 0;
    virtual void progressTime(int newTime) = 0;
    virtual ~Runner();
};

#endif
