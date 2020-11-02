#include <cassert>

#include "Device.h"

Device::Device() {
    setFree();
}

void Device::setNextFree(int nextFree) {
    this->nextFree = nextFree;
}

int Device::getNextFree() {
    return nextFree;
}

void Device::setState(int state) {
    this->state = state;
}

void Device::setBusyWait() {
    setState(1);
    setNextFree(2e9 + 10);
}

void Device::setBusy(int nextFree) {
    setState(2);
    setNextFree(nextFree);
}

void Device::setFree() {
    setState(0);
    setNextFree(-1);
}

bool Device::isFree() {
    return state == 0;
}

bool Device::isBusyWait() {
    return state == 1;
}

bool Device::isBusy() {
    return state == 2;
}

void Device::refresh(int curTime) {
    if (isBusy() && curTime >= getNextFree()) {
        setFree();
    }
}

int Device::getState() {
    return state;
}
