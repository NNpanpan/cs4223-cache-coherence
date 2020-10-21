#ifndef BUSUSER_H
#define BUSUSER_H

/*
Class BusUser:
    - Represents an entity that snoops off the common data bus.
*/

class BusUser {
public:
    int busUpdate(const BusUser & requestor, const int & addr) const;
    int busRd(const BusUser & requestor, const int & addr) const;
    int busRdX(const BusUser & requestor, const int & addr) const;
    
    virtual int hasEntry(const int & addr) = 0;
    virtual int flush(const int & addr) = 0;
};

#endif