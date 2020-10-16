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

void Cache::updateState(const int & addr, string new_state) {
    int setNum = (addr / blockSize) % setCount;
    int posInBlock = addr % blockSize;
    auto set = entries[setNum];
    for (auto block : set) {
        if (block.second[posInBlock] == addr) {
            block.first = new_state;
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