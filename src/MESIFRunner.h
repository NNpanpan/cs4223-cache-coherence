#ifndef MESIFRUNNER_H
#define MESIFRUNNER_H

#include <map>

#include "Runner.h"

/*
 * Extends classical non-intervention MESI to allow intervention from both M and
 * E states, then extends that to include an additional forwarding F state.
 */
class MESIFRunner : public Runner {
private:
    const string NO_FORWARDER = "MEM";

    /*
     * Returns the number of other copies of the cache block, and the state of
     * the forwarding cache if it exists (e.g. any cache in M, E, S or F state).
     */
    pair<int, string> getCopiesWithForwarder(int addr);

    /*
     * Obtains a memory block either from another cache or main memory.
     */
    bool fetchAndAlloc(int cacheID, int addr,
        string newState, string forwarderState);

    /*
     * Invalidates all copies of the block in other caches - if needWriteback is
     * true, any line invalidated in 'M' state will trigger a Flush (write-back)
     * of the dirty line back to memory.
     */
    void invalidateO(int cacheID, int addr, bool needWriteBack);

protected:
    void progressTime(int newTime) override;

public:
    MESIFRunner(int cacheSize, int assoc, int blockSize,
        vector<vector<pair<int, int>>> coreTraces);

    /*
     * Simulates a read hit in a given cache in F, S, E or M states.
     */
    void simulateReadHit(int coreID, int addr) override;
    /*
     * Simulates changes and invalidations to cache states across all caches
     * due to a read hit in either F, S, E or M states. Write hit is deferred
     * if the block is involved in an outstanding bus request.
     */
    void simulateWriteHit(int coreID, int addr) override;
    /*
     * Simulates changes to cache states across all caches due to a read miss
     * in I state, including block fetch (memory/C2C transfer) and additional
     * Flush (write-back) if another cache has block in M state.
     */
    void simulateReadMiss(int coreID, int addr) override;
    /*
     * Simulates changes to cache states across all caches due to a write miss
     * in I state. Write miss is deferred if the block is involved in an
     * outstanding bus request.
     */
    void simulateWriteMiss(int coreID, int addr) override;
    ~MESIFRunner();
};

#endif

