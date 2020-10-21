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
private:
    vector<BusUser*> busUsers; // all BusUsers
    int userCount;
    int trafficData;
    int invalidateCount;
    int updateCount;

public:
    Bus(const int & userCount);

    void addBusUser(BusUser* busUser, const int & pos);

    void incrTrafficData(const int & amount);
    int getTrafficData();
    void incrInvalidateCount();
    int getInvalidateCount();
    void incrUpdateCount();
    int getUpdateCount();

    int findEntry(const int & addr);
};

#endif