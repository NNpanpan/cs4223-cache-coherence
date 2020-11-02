#include <string>
#include <cassert>

#include "Core.h"

using namespace std;

Core::Core(vector<pair<int, int>> traces, int ID) : Device() {
    this->ID = ID;
    for(auto trace : traces) {
        traceQ.push(trace);
    }
}

bool Core::isFinish() {
    return isFree() && traceQ.empty();
}

pair<int, int> Core::peekTrace() {
    assert(!traceQ.empty());
    return traceQ.front();
}

void Core::popTrace() {
    assert(!traceQ.empty());
    traceQ.pop();
}

int Core::getID() {
    return ID;
}

void Core::incLoadCount() {
    loadCount++;
}

int Core::getLoadCount() {
    return loadCount;
}

void Core::incStoreCount() {
    storeCount++;
}

int Core::getStoreCount() {
    return storeCount;
}

void Core::incCacheMissCount() {
    cacheMissCount++;
}

int Core::getCacheMissCount() {
    return cacheMissCount;
}

void Core::incPrivateAccessCount() {
    privateAccessCount++;
}

int Core::getPrivateAccessCount() {
    return privateAccessCount;
}

int Core::getIdleCycles() {
    return idleCycles;
}

void Core::incIdleCycles(int cycles) {
    idleCycles += cycles;
}

int Core::getExecCycles() {
    return execCycles;
}

void Core::incExecCycles(int cycles) {
    execCycles += cycles;
}

int Core::getCompCycles() {
    return compCycles;
}

void Core::incCompCycles(int cycles) {
    compCycles += cycles;
}

void Core::progress(int cycles) {
    if (isFinish()) {
        return; ///freeze finished core
    }

    // assert(cycles > 0);
    incExecCycles(cycles); /// stat 1
    if (isBusyWait()) incIdleCycles(cycles); /// stat 4
    if (isBusy()) incCompCycles(cycles); /// stat 2
}
