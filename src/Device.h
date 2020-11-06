#ifndef DEVICE_H
#define DEVICE_H

#include <cassert>
/*
Class Device:
    - Base class of all object
    - Implement some base state
 */


class Device{
private:
    int state; /// 0 for free, 1 for busy_wait, 2 for busy
    int nextFree;

    void setState(int state);
    void setNextFree(int nextFree);
public:
    Device();

    void setBusyWait(); ///set nextFree to INF and state to busy_wait
    void setBusy(int nextFree); ///set this device busy until nextFree
    void setFree(); ///set nextFree to -1 and state to free

    bool isFree();
    bool isBusyWait();
    bool isBusy();

    int getNextFree();
    void refresh(int curTime);
};

#endif // DEVICE_H
