#include <cassert>

#include "CacheReq.h"

CacheReq::CacheReq(int cacheID, int addr, int initTime, string reqType) {
    /// very lazy
    assert(reqType == "rd" || reqType == "rdX" || reqType == "wr" || reqType == "upd");

    this->cacheID = cacheID;
    this->addr = addr;
    this->initTime = initTime;
    this->reqType = reqType;
}

int CacheReq::getCacheID() const {
    return cacheID;
}

int CacheReq::getInitTime() const {
    return initTime;
}

int CacheReq::getAddr() const {
    return addr;
}

string CacheReq::getReqType() const {
    return reqType;
}

bool CacheReq::operator < (const CacheReq &oth) const {
    if (initTime != oth.initTime)
        return initTime < oth.initTime;
    if (cacheID != oth.cacheID) {
        return cacheID < oth.cacheID;
    }

    if (addr != oth.addr) {
        return addr < oth.addr;
    }

    if (reqType != oth.reqType) {
        return reqType < oth.reqType;
    }

    return 0;
}
