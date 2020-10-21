#ifndef MESIBUS_H
#define MESIBUS_H

#include "Bus.h"

/*
Class MESIBus:
    - Represents the data bus that adheres to MESI protocol.
*/

class MESIBus : public Bus {
public:
    /*
    Both busRd and busRdX return 1 if some other cache has data,
    return 0 otherwise
    */
    int busRd(const int & addr); 
    int busRdX(const int & addr);
};

#endif