#ifndef BUS_H
#define BUS_H

#include <vector>
#include <set>

using namespace std;

/*
Class Bus:
    - The common data bus
*/

class Bus {
private:
    int blockSize;
    int trafficData; /// stat 6
    int invalidateCount; /// stat 7
    int updateCount; /// stat 7


public:
    Bus(int blockSize);

    int getWordPerBlock();

    int getTrafficData();
    void incTrafficBlock(int numBlock = 1);
    void incTrafficWord(int numWord = 1);

    void incInvalidateCount();
    int getInvalidateCount();
    void incUpdateCount();
    int getUpdateCount();
};

#endif
