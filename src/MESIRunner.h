#ifndef MESIRUNNER_H
#define MESIRUNNER_H

#include "Runner.h"

class MESIRunner : public Runner {
private:
    bool cacheAllocAddr(int cacheID, int addr, string state);
    void invalidateO(int cacheID, int addr);
public:
    MESIRunner(int cacheSize, int assoc, int blockSize,
        vector<vector<pair<int, int>>> coreTraces);
    void execCacheReq(CacheReq req) override;
    void simulateReadHit(int coreID, int addr) override;
    void simulateWriteHit(int coreID, int addr) override;
    void simulateReadMiss(int coreID, int addr) override;
    void simulateWriteMiss(int coreID, int addr) override;
};

#endif
