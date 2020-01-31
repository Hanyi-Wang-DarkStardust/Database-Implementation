
#ifndef BUFFER_MGR_C
#define BUFFER_MGR_C

#include "MyDB_BufferManager.h"
#include <string>
#include "zconf.h"

using namespace std;


// global timestamp, auto decrease when creating new pages or updating/accessing a page (like read)
int TimeStamp = 0;

MyDB_PageHandle MyDB_BufferManager :: getPage (MyDB_TablePtr table , long id) {
    TimeStamp--; // new timestamp, timestamp is always unique
    pair<MyDB_TablePtr, long> params = make_pair(table, id);
    auto it = pageMap.find(params);
    if (it == pageMap.end()) {
        if (bufferPool.empty()) { // buffer is full, so need to evict one page
            evictPage();
        }
        void *emptyPoolBuffer = bufferPool.back(); // assign one empty slot from buffer
        bufferPool.pop_back();
        MyDB_PagePtr pagePtr = make_shared<MyDB_Page>(table, id, pageSize, tempFile,  this, emptyPoolBuffer, false, false);
        pagePtr->readDataFromFile(); // try to read its raw data from disk/file
        pageMap.insert(make_pair(params, pagePtr));
        lru->append(pagePtr);
        return make_shared<MyDB_PageHandleBase>(pagePtr);
    }
    else {
//        it->second->unpinPage();
        return make_shared<MyDB_PageHandleBase>(it->second);
    }
}

MyDB_PageHandle MyDB_BufferManager :: getPage () {
    TimeStamp--;
    pair<MyDB_TablePtr, long> params = make_pair(nullptr, counterAnonymous);
    if (bufferPool.empty()) {
        evictPage();
    }
    void *emptyPoolBuffer = bufferPool.back();
    bufferPool.pop_back();
    MyDB_PagePtr newAnonymousPage = make_shared<MyDB_Page>(nullptr, counterAnonymous, pageSize, tempFile, this, emptyPoolBuffer, false, true);
    pageMap.insert(make_pair(params, newAnonymousPage));
    lru->append(newAnonymousPage);
    counterAnonymous++;
	return make_shared<MyDB_PageHandleBase>(newAnonymousPage);
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage (MyDB_TablePtr table, long id) {
    TimeStamp--;
    pair<MyDB_TablePtr, long> params = make_pair(table, id);
    auto it = pageMap.find(params);
    if (it == pageMap.end()) {
        if (bufferPool.empty()) {
            evictPage();
        }
        void *emptyPoolBuffer = bufferPool.back();
        bufferPool.pop_back();
        MyDB_PagePtr pagePtr = make_shared<MyDB_Page>(table, id, pageSize, tempFile,  this, emptyPoolBuffer, true, false);
        pageMap.insert(make_pair(params, pagePtr));
        lru->append(pagePtr);
        return make_shared<MyDB_PageHandleBase>(pagePtr);
    }
    else {
        it->second->pinPage(); // set current un-pinned page to pinned
        return make_shared<MyDB_PageHandleBase>(it->second);
    }
}

MyDB_PageHandle MyDB_BufferManager :: getPinnedPage () {
    TimeStamp--;
    pair<MyDB_TablePtr, long> params = make_pair(nullptr, counterAnonymous);
    if (bufferPool.empty()) {
        evictPage();
    }
    void *emptyPoolBuffer = bufferPool.back();
    bufferPool.pop_back();
    MyDB_PagePtr newAnonymousPage = make_shared<MyDB_Page>(nullptr, counterAnonymous, pageSize, tempFile, this, emptyPoolBuffer, true, true);
    pageMap.insert(make_pair(params, newAnonymousPage));
    lru->append(newAnonymousPage);
    counterAnonymous++;
    return make_shared<MyDB_PageHandleBase>(newAnonymousPage);
}

void MyDB_BufferManager :: unpin (MyDB_PageHandle unpinMe) {
    unpinMe->getPage()->unpinPage();
}


void MyDB_BufferManager :: evictPage() {
    MyDB_PagePtr popPage = lru->popTail(); // get oldest un-pinned one
    if (popPage == nullptr) {
        cout << "Try to pop empty LRU! Exit program"<<endl;
        exit(EXIT_FAILURE);
    }
    if (popPage->getDirty()) {
        popPage->writeBackToFile();
    }
    pageMap.erase(popPage->getTablePair());
    bufferPool.push_back(popPage->getRawData());
}

MyDB_BufferManager :: MyDB_BufferManager (size_t pageSize, size_t numPages, string tempFile) {
    this->pageSize = pageSize;
    this->numPages = numPages;
    this->tempFile = tempFile;
    counterAnonymous = 0;
    buffer = (void *) malloc(numPages * pageSize); // all available buffer
    void *p =  buffer;
    for (int i = 0; i < numPages; i++) {  // slice buffer
        bufferPool.push_back(p);
        p = (void *) ((char *) p + pageSize);  // since each char is one byte, so increment is also one byte
    }
    lru = new LRU(numPages);
    TimeStamp = 0; // reset global timestamp
}

MyDB_BufferManager :: ~MyDB_BufferManager () {
    for (auto& it : pageMap) {
        if (!it.second->getPin()) {
            killPage(it.second); // kill all un-pinned pages to write back
        }
    }
    free(buffer);
    delete lru;
}


map<pair<MyDB_TablePtr, long>, MyDB_PagePtr> MyDB_BufferManager :: getPageMap() {
    return pageMap;
}

string MyDB_BufferManager :: getTempFile() {
    return tempFile;
}

LRU * MyDB_BufferManager :: getLRU() {
    return lru;
}


void MyDB_BufferManager :: killPage(MyDB_PagePtr page) {
    if (page->getDirty()) {
        page->writeBackToFile();
    }
    pageMap.erase(page->getTablePair());
    bufferPool.push_back(page->getRawData());
}

#endif


