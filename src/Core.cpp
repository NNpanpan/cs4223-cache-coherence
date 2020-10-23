#include <string>

#include "Cache.h"
#include "Core.h"

using namespace std;

Core::Core(Cache* cache_ptr, const int & id) {
    cache = cache_ptr;
    ID = id;
}

int Core::getID() const {
    return ID;
}

void Core::setExecCycles(const int & cycles) {
    execCycles = cycles;
}

void Core::setCompCycles(const int & cycles) {
    compCycles = cycles;
}

void Core::setIdleCycles(const int & cycles) {
    idleCycles = cycles;
}

void Core::setLSInstCount(const int & cnt) {
    loadStoreInstCount = cnt;
}

void Core::incrExecCycles(const int & cycles) {
    execCycles += cycles;
}

int Core::getExecCycles() {
    return execCycles;
}

void Core::incrCompCycles(const int & cycles) {
    compCycles += cycles;
}

int Core::getCompCycles() {
    return compCycles;
}

void Core::incrIdleCycles(const int & cycles) {
    idleCycles += cycles;
}

int Core::getIdleCycles() {
    return idleCycles;
}

void Core::incrLSInstCount() {
    loadStoreInstCount++;
}

int Core::getLSInstCount() {
    return loadStoreInstCount;
}

int Core::execCmd(const int & cmdType, const int & info) {
    switch (cmdType) {
        case 0:
            int cost = prRd(info);
            incrIdleCycles(cost);
            incrExecCycles(cost);
            setNextFreeCycle(execCycles);
            return cost;
        case 1:
            int cost = prWr(info);
            incrIdleCycles(cost);
            incrExecCycles(cost);
            setNextFreeCycle(execCycles);
            return cost;
        case 2:
            int cost = computeOthers(info);
            incrCompCycles(cost);
            incrExecCycles(cost);
            setNextFreeCycle(execCycles);
            return cost;
        default:
            return 0;
    }
}

int Core::computeOthers(const int & cycles) {
    isFree = 0;
    
    return cycles;
}

int Core::prRd(const int & addr) {
    isFree = 0;
    return cache->prRd(addr);
}

int Core::prWr(const int & addr) {
    isFree = 0;
    return cache->prWr(addr);
}

void Core::finaliseStats() {
    idleCycles = execCycles - compCycles;
}
