#include "BusUser.h"
using namespace std;

int BusUser::busUpdate(const BusUser & requestor, const int & addr) const {
    return -1;
}

int BusUser::busRd(const BusUser & requestor, const int & addr) const {
    return -1;
}

int BusUser::busRdX(const BusUser & requestor, const int & addr) const {
    return -1;
}

int BusUser::flush(const int & addr) const {
    // number of cycles to go to memory
    
    return 100;
}