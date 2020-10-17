#include <string>
#include <utility>
#include <vector>

#include "Bus.h"
#include "BusUser.h"
#include "Cache.h"

using namespace std;

Cache::Cache(const int & assoc, const int & blockSize, const int & cacheSize) {
    this->associativity = assoc;
    this->blockSize = blockSize;
    this->cacheSize = cacheSize;
    this->entries = vector<vector<pair<string, vector<int>>>>();

    int setCount = cacheSize / (blockSize * assoc);
    this->setCount = setCount;
    for (int i = 0; i < setCount; i++) {
        auto set = vector<pair<string, vector<int>>>();
        for (int j = 0; j < assoc; j++) {
            auto block = vector<int>(blockSize/4);
            set.push_back(pair<string, vector<int>>("I", block));
        }
        this->entries.push_back(set);
    }
}

void Cache::allocEntry(const int & addr) {
    // Only allocate entry. State is changed to "V" - for valid.
    // This "V" state is just a placeholder.
    // Actual state depends on the protocol.
    // The coherence protocol will decide which state to set.
    int setNum = (addr / blockSize) % setCount;
    int posInBlock = addr % blockSize;
    auto set = entries[setNum];
    int avail = 0;
    for (auto block : set) {
        if (block.first == "I") {
            block.second[posInBlock] = addr;
            for (int j = 0; j < blockSize/4; j++) {
                block.second[j] = addr + (j - posInBlock);
            }
            avail = 1;
            block.first = "V";
            break;
        }
    }

    if (!avail) {
        auto lruBlock = set[lastUsed[setNum]];
        for (int j = 0; j < blockSize/4; j++) {
            lruBlock.second[j] = addr + (j - posInBlock);
        }
        lruBlock.first = "V";
    }
}

int Cache::hasEntry(const int & addr) const {
    // Returns 1 if addr is cached, 0 otherwise
    int setNum = (addr / blockSize) % setCount;
    int posInBlock = addr % blockSize;
    auto set = entries[setNum];
    for (auto block : set) {
        if (block.first != "I" && block.second[posInBlock] == addr)
            return 1;
    }
    
    return 0;
}

void Cache::updateState(const int & addr, string newState) {
    int setNum = (addr / blockSize) % setCount;
    int posInBlock = addr % blockSize;
    auto set = entries[setNum];
    for (auto block : set) {
        if (block.second[posInBlock] == addr) {
            block.first = newState;
            break;
        }
    }
}

void Cache::setBusUser(BusUser* busUser) {
    this->busUser = busUser;
}

int Cache::prRd(const int & addr) const {
    return busUser->busRd(*busUser, addr);
}

int Cache::prWr(const int & addr) const {
    return busUser->busRdX(*busUser, addr);
}

int Cache::flush(const int & addr) const {
    return busUser->flush(addr);
}