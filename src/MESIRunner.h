#ifndef MESIRUNNER_H
#define MESIRUNNER_H

#include <map>

#include "Runner.h"

class MESIRunner : public Runner {
private:
    bool cacheAllocAddr(int cacheID, int addr, string state);

    // If needWriteBack is true, line being invalidated in 'M' state
    // Requires write-back of dirty block to mem
    void invalidateO(int cacheID, int addr, bool needWriteBack);
protected:
    void progressTime(int newTime) override;
public:
    MESIRunner(int cacheSize, int assoc, int blockSize,
        vector<vector<pair<int, int>>> coreTraces);
    void simulateReadHit(int coreID, int addr) override;
    void simulateWriteHit(int coreID, int addr) override;
    void simulateReadMiss(int coreID, int addr) override;
    void simulateWriteMiss(int coreID, int addr) override;
    ~MESIRunner();
};

#endif

