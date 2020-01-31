//
// Created by Hanyi Wang on 1/25/20.
//

#ifndef LRU_H
#define LRU_H

#include <iostream>
#include "MyDB_Page.h"
#include <queue>
using namespace std;

extern int TimeStamp;

class MyDB_BufferManager;


class LRU {

public:
    // Constructor
    LRU(int capacity);

    // Destructor
    ~LRU();

    // add a new page into LRU
    void append(MyDB_PagePtr page);

    // pop oldest un-pinned page
    MyDB_PagePtr popTail();

    // update LRU (page may have a new timestamp)
    void update(MyDB_PagePtr page);

    // LRU is full?
    bool isFull();


private:
    int capacity;

    // LRU
    priority_queue<pair<int, MyDB_PagePtr>> heap;

};

#endif //LRU_H