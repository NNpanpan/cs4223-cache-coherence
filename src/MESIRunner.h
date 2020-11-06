#ifndef MESIRUNNER_H
#define MESIRUNNER_H

#include <map>

#include "Runner.h"

class MESIRunner : public Runner {
private:
    static const int INF = 2e9 + 10;
    map<int, int> invalidBlock; /// store block that memory is not holding and first moment when it is available

    int getMemBlockAvailableTime(int blockNum);
    void cacheWriteBackMem(int cacheID, int addr);
    bool cacheAllocAddr(int cacheID, int addr, string state);

    /// if needWriteBack is true, then cache in 'M' state need to write back to mem
    void invalidateO(int cacheID, int addr, bool needWriteBack);

    void checkMem();
public:
    MESIRunner(int cacheSize, int assoc, int blockSize,
        vector<vector<pair<int, int>>> coreTraces);
    //bool execCacheReq(CacheReq req) override;
    void simulateReadHit(int coreID, int addr) override;
    void simulateWriteHit(int coreID, int addr) override;
    void simulateReadMiss(int coreID, int addr) override;
    void simulateWriteMiss(int coreID, int addr) override;
    void progressTime(int newTime) override;
    ~MESIRunner();
};

#endif
