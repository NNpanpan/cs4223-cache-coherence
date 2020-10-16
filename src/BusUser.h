#ifndef BUSUSER_H
#define BUSUSER_H

/*
Class BusUser:
    - Represents an entity that snoops off the common data bus.
*/

class BusUser {
public:
    int BusUpdate(const BusUser & requestor, const int & addr) const;
    int BusRd(const BusUser & requestor, const int & addr) const;
    int BusRdX(const BusUser & requestor, const int & addr) const;
    int flush(const int & addr) const;
};

#endif