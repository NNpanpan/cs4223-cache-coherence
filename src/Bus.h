#ifndef BUS_H
#define BUS_H

#include <vector>
#include "BusUser.h"

using namespace std;

/*
Class Bus:
    - The common data bus.
*/

class Bus {
public:
    vector<BusUser> busUsers; // all BusUsers
    int userCount;
    int trafficData;
    int invalidateCount;
    int updateCount;

    Bus(const int & userCount);
    void addBusUser(const BusUser & busUser);
    int busUpdate(const BusUser & requestor, const int & addr) const;
    int busRd(const BusUser & requestor, const int & addr) const;
    int busRdX(const BusUser & requestor, const int & addr) const; // should invalidate
    int flush(const int & addr) const;
};

#endif