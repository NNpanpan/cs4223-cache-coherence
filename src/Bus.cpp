#include <vector>
#include "Bus.h"

using namespace std;

Bus::Bus(const int & userCount) {
    this->userCount = userCount;
    this->busUsers = vector<BusUser*>(userCount);
}

void Bus::addBusUser(BusUser* busUser, const int & pos) {
    busUsers[pos] = busUser;
}

void Bus::incrTrafficData(const int & amount) {
    trafficData += amount;
}

int Bus::getTrafficData() {
    return trafficData;
}

void Bus::incrInvalidateCount() {
    invalidateCount++;
}

int Bus::getInvalidateCount() {
    return invalidateCount;
}

void Bus::incrUpdateCount() {
    updateCount++;
}

int Bus::getUpdateCount() {
    return updateCount;
}

int Bus::findEntry(const int & addr) {
    for (auto bU : busUsers) {
        if (bU->hasEntry(addr)) {
            return 1;
        }
    }

    return 0;
}