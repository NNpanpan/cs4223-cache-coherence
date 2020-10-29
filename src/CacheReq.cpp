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

int CacheReq::getCacheID() {
    return cacheID;
}

int CacheReq::getInitTime() {
    return initTime;
}

int CacheReq::getAddr() {
    return addr;
}

string CacheReq::getReqType() {
    return reqType;
}

bool CacheReq::operator < (CacheReq &oth) {
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
