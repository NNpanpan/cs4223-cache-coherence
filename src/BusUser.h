#ifndef BUSUSER_H
#define BUSUSER_H

/*
Class BusUser:
    - Represents an entity that snoops off the common data bus.
*/

class BusUser {
public:
    virtual int hasEntry(const int & addr) const = 0;
    virtual int flush(const int & addr) = 0;
};

#endif
