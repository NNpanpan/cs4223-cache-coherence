#ifndef RUNNER_H
#define RUNNER_H

#include <map> 

#include "Core.h"
#include "Bus.h"
#include "Cache.h"

using namespace std;

class Runner {
protected:
    int curTime;

    vector<Core> cores;
    vector<Cache> caches;
    Bus bus;

    static const int INF = 2e9 + 10;
    // Store blocks that memory is not holding and first moment when it is available
    map<int, int> invalidBlock;

    void setMemBlockAvailableTime(int blockNum, int availTime);
    void setMemBlockUnavailable(int blockNum);
    int getMemBlockAvailableTime(int blockNum);
    void cacheWriteBackMem(int cacheID, int addr);
    void checkMem();

    void printStat();
    bool isAllFinish();

    bool checkReleaseCore();
    bool checkCoreReq();
    void progressTime(int newTime);

    void printDebug();

public:
    Runner(int cacheSize, int assoc, int blockSize,
        vector<vector<pair<int, int>>> coreTraces);

    void simulate();
    // virtual bool execCacheReq(CacheReq req) = 0;

    // Note: these function use curTime, so have not counted 1 cycle for cache check yet
    virtual void simulateReadHit(int coreID, int addr) = 0;
    virtual void simulateWriteHit(int coreID, int addr) = 0;
    virtual void simulateReadMiss(int coreID, int addr) = 0;
    virtual void simulateWriteMiss(int coreID, int addr) = 0;
    virtual ~Runner();
};

#endif

