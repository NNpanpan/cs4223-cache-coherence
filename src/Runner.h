#ifndef RUNNER_H
#define RUNNER_H

#include <map> 

#include "Core.h"
#include "Bus.h"
#include "Cache.h"

using namespace std;

class Runner {
protected:
    bool earlyRet;

    int curTime;

    int blockSize;
    
    vector<Core> cores;
    vector<Cache> caches;
    Bus bus;

    static const int INF = 2e9 + 10;
    // Store blocks that memory is not holding and first moment when it is available
    map<int, int> invalidBlock;
    // Store activeBlocks and their request ID.
    map<int, int> activeBlocks;

    int getHeadAddr(int addr);

    int getMemBlockAvailableTime(int blockNum);
    void cacheWriteBackMem(int cacheID, int addr);
    void checkMem();

    void printStat();
    bool isAllFinish();

    bool checkReleaseCore();
    bool checkCoreReq();
    virtual void progressTime(int newTime) = 0;

    void printDebug();

public:
    Runner(int cacheSize, int assoc, int blockSize,
        vector<vector<pair<int, int>>> coreTraces);

    void simulate();
    // virtual bool execCacheReq(CacheReq req) = 0;

    virtual void simulateReadHit(int coreID, int addr) = 0;
    virtual void simulateWriteHit(int coreID, int addr) = 0;
    virtual void simulateReadMiss(int coreID, int addr) = 0;
    virtual void simulateWriteMiss(int coreID, int addr) = 0;
    virtual ~Runner();
};

#endif

