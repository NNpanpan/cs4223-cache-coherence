#include <fstream>
#include <string>

#include "Cache.h"
#include "Core.h"

using namespace std;

Core::Core(const string & filepath, Cache* cache_ptr) {
    ifs.open(filepath);
    cache = cache_ptr;
}

Core::~Core() {
    ifs.close();
}

int Core::incrCycles(const int & cycles) {
    execCycles += cycles;
}

int Core::computeOthers(const int & cycles) {
    compCycles += cycles;
    isFree = 0;
    nextFreeCycle = execCycles + cycles;
    return cycles;
}

int Core::prRd(const int & addr) {
    loadStoreInstCount++;
    return cache->prRd(addr);
}

int Core::prWr(const int & addr) {
    loadStoreInstCount++;
    return cache->prWr(addr);
}

void Core::finaliseStats() {
    idleCycles = execCycles - compCycles;
}