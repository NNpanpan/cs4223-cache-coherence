#ifndef CACHEREQ_H
#define CACHEREQ_H

#include <string>

using namespace std;

/*
   Base cache request to bus
*/

/// this assume that state change of block cost 0 cycle

class CacheReq {
private:
    int cacheID; ///which cache it originated from
    int addr; /// to which address
    int initTime; /// for priority
    string reqType; ///can either be "rd", "rdX", "wr"(flush), "upd"
public:
    CacheReq(int cacheID, int addr, int initTime, string reqType); ///can be more safer using Factory
    int getInitTime() const;
    int getCacheID() const;
    int getAddr() const;
    string getReqType() const ;

    bool operator < (const CacheReq &oth) const;
};

#endif
