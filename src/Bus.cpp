#include <vector>
#include "Bus.h"

using namespace std;

Bus::Bus(int blockSize) {
    this->blockSize = blockSize;

    trafficData = 0;
    invalidateCount = 0;
    updateCount = 0;
}


int Bus::getTrafficData() {
    return trafficData;
}

void Bus::incTrafficBlock(int numBlock) {
    trafficData += numBlock * blockSize;
}

void Bus::incTrafficWord(int numWord) {
    trafficData += 4 * numWord; ///default word size
}

void Bus::incInvalidateCount() {
    invalidateCount++;
}

int Bus::getInvalidateCount() {
    return invalidateCount;
}

void Bus::incUpdateCount() {
    updateCount++;
}

int Bus::getUpdateCount() {
    return updateCount;
}
