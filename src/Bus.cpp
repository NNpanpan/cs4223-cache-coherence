#include <vector>
#include "Bus.h"

using namespace std;

Bus::Bus(const int & userCount) {
    this->userCount = userCount;
    this->busUsers = vector<BusUser>(userCount);
}

void Bus::addBusUser(const BusUser & busUser) {
    (this->busUsers).push_back(busUser);
}

int Bus::busUpdate(const BusUser & requestor, const int & addr) const {
    return -1;
}

int Bus::busRd(const BusUser & requestor, const int & addr) const {
    return -1;
}

int Bus::busRdX(const BusUser & requestor, const int & addr) const {
    return -1;
}

int Bus::flush(const int & addr) const {
    // May return the number of cycles to access memory
    return 100;
}