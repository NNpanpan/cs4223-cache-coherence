#include <fstream>
#include <string>

#include "Cache.h"
#include "Core.h"

using namespace std;

Core::Core(Cache* cache_ptr) {
    cache = cache_ptr;
}

int Core::incrCycles(const int & cycles) {
    execCycles += cycles;
    return cycles;
}

int Core::execCmd(const int & cmdType, const int & info) {
    switch (cmdType) {
        case 0:
            int cost = prRd(info);
            nextFreeCycle = execCycles + cost;
            return cost;
        case 1:
            int cost = prWr(info);
            nextFreeCycle = execCycles + cost;
            return cost;
        case 2:
            int cost = computeOthers(info);
            nextFreeCycle = execCycles + cost;
            return cost;
        default:
            return -1;
    }
}

int Core::computeOthers(const int & cycles) {
    compCycles += cycles;
    isFree = 0;
    
    return cycles;
}

int Core::prRd(const int & addr) {
    loadStoreInstCount++;
    isFree = 0;
    return cache->prRd(addr);
}

int Core::prWr(const int & addr) {
    loadStoreInstCount++;
    isFree = 0;
    return cache->prWr(addr);
}

void Core::finaliseStats() {
    idleCycles = execCycles - compCycles;
}