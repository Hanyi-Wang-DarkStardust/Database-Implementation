//
// Created by Hanyi Wang on 1/25/20.
//

#ifndef LRU_CC
#define LRU_CC

#include "LRU.h"
#include "MyDB_BufferManager.h"
using namespace std;



LRU :: LRU(int capacity) {
    this->capacity = capacity;
}

LRU :: ~LRU() {

}


void LRU :: append(MyDB_PagePtr page) {
    auto pagePair = make_pair(TimeStamp, page); // timestamp is a global variable
    heap.push(pagePair);
}

MyDB_PagePtr LRU :: popTail() {
    vector<pair<int, MyDB_PagePtr>> store;
    while (heap.top().second->getPin()) { // do not pop pinned page
        store.push_back(heap.top());
        heap.pop();
    }
    if (heap.empty()) { // all pages in LRU are pinned
        cout << "All pages in LRU are pinned! Exit program" << endl;
        exit(EXIT_FAILURE);
    }
    auto popPair = heap.top();
    heap.pop();
    for (const auto& it : store) {
        heap.push(it);
    }
    return popPair.second;
}

void LRU :: update(MyDB_PagePtr page) { // update new timestamp, and so lead to new order in heap(LRU)
    vector<pair<int, MyDB_PagePtr>> store;
    while (page != heap.top().second) {
        store.push_back(heap.top());
        heap.pop();
    }
    heap.pop();
    heap.push(make_pair(TimeStamp, page)); // use a new TimeStamp
    for (const auto& it : store) {
        heap.push(it);
    } // heap can adjust order automatically
}

bool LRU :: isFull() {
    return heap.size() == capacity;
}


#endif //LRU_CC