#ifndef CORE_H
#define CORE_H

#include <queue>
#include <vector>

#include "Device.h"

using namespace std;

/*
Class Core:
    - Represents the processor
*/

class Core : public Device {
private:
    queue<pair<int, int>> traceQ;
    int ID;

    /// stat related
    int execCycles; /// stat 1
    int compCycles; /// stat 2
    int idleCycles; /// stat 4

    int loadCount;  /// stat 3
    int storeCount; /// stat 3
    int cacheMissCount; ///miss rate = cacheMissCount / (loadCount + storeCount) stat 5
    int privateAccessCount; /// sharedAccess = loadCount - privateAccessCount stat 8

public:
    Core(vector<pair<int, int>> traces, int ID);
    bool isFinish();

    pair<int, int> peekTrace(); /// return the first trace
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
