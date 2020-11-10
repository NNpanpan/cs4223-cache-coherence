#ifndef CORE_H
#define CORE_H

#include <queue>
#include <vector>

#include "Device.h"

using namespace std;

/*
 *  Class Core:
 *  - Represents the processor
 */
class Core : public Device {
public: /// TBD : rechange to private, debugging
    queue<pair<int, int>> traceQ;
    int ID;

    // For tracking execution statistics
    int execCycles;         // Stat 1
    int compCycles;         // Stat 2
    int idleCycles;         // Stat 4

    int loadCount;          // Stat 3
    int storeCount;         // Stat 3

    // Miss rate = cacheMissCount / (loadCount + storeCount)
    int cacheMissCount;     // Stat 5
    // sharedAccess = loadCount - privateAccessCount
    int privateAccessCount; // Stat 8

public:
    Core(vector<pair<int, int>> traces, int ID);
    bool isFinish();

    pair<int, int> peekTrace();     // Returns the first trace
    void popTrace();

    int getID();

    void incLoadCount();
    int getLoadCount();

    void incStoreCount();
    int getStoreCount();

    void incCacheMissCount();
    int getCacheMissCount();

    void incPrivateAccessCount();
    int getPrivateAccessCount();

    int getExecCycles();
    void incExecCycles(int cycles);
    int getIdleCycles();
    void incIdleCycles(int cycles);
    int getCompCycles();
    void incCompCycles(int cycles);

    void progress(int cycles);
};

#endif

