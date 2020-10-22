#include <string>
#include <utility>
#include <vector>

#include "Bus.h"
#include "BusUser.h"
#include "Cache.h"

using namespace std;

Cache::Cache(const int & assoc, const int & blockSize, const int & cacheSize, const int & ID) {
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

int Cache::getID() const {
    return ID;
}

void Cache::incrHit() {
    hitNum++;
}

int Cache::getHitNum() {
    return hitNum;
}

void Cache::incrTotalRq() {
    totalRq++;
}

int Cache::getTotalRq() {
    return totalRq;
}

void Cache::incrPrivateHits() {
    privateHits++;
}

int Cache::getPrivateHits() {
    return privateHits;
}

void Cache::incrSharedHits() {
    sharedHits++;
}

int Cache::getSharedHits() {
    return sharedHits;
}

int Cache::getBlockWordCount() {
    return blockSize/4;
}

void Cache::setLastUsed(const int & addr) {
    int setNum = (addr / blockSize) % setCount;
    int posInBlock = addr % blockSize;
    auto set = entries[setNum];
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

int Cache::updateState(const int & addr, string newState) {
    int setNum = (addr / blockSize) % setCount;
    int posInBlock = addr % blockSize;
    auto set = entries[setNum];
    for (auto block : set) {
        if (block.second[posInBlock] == addr) {
            block.first = newState;
            return 1;
        }
    }

    // Does not find block. Something's wrong
    return 0;
}

int Cache::prRd(const int & addr) {
    // Placeholder
    return -1;
}

int Cache::prWr(const int & addr) {
    // Placeholder
    return -1;
}

int Cache::flush(const int & addr) {
    // if block exists, vacate successfully and access memory
    // return 100 - memory access cost
    // otherwise, return 0
    return updateState(addr, "I") ? 100 : 0;
}
