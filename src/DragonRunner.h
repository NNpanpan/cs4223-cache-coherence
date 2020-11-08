#ifndef DRAGONRUNNER_H
#define DRAGONRUNNER_H

#include <map>

#include "Runner.h"

/// use 4 state: M, Sc, Sm, E
class DragonRunner : public Runner {
private:

    int countOthCacheHold(int cacheID, int addr);
    void cacheReceiveW(int cacheID, int addr, int sendCycle);
    void cacheReceiveB(int cacheID, int addr, string state);
    int findMemSourceAvailableTime(int cacheID, int addr);
    int findCacheSourceAvailableTime(int cacheID, int addr);
    int findSourceAvailableTime(int cacheID, int addr);
    void broadcastWOthCache(int cacheID, int addr, int sendCycle);
public:
    DragonRunner(int cacheSize, int assoc, int blockSize,
        vector<vector<pair<int, int>>> coreTraces);
    void simulateReadHit(int coreID, int addr) override;
    void simulateWriteHit(int coreID, int addr) override;
    void simulateReadMiss(int coreID, int addr) override;
    void simulateWriteMiss(int coreID, int addr) override;
    ~DragonRunner();
};

#endif
