#ifndef PAGE_HANDLE_C
#define PAGE_HANDLE_C

#include <memory>
#include "MyDB_BufferManager.h"

void *MyDB_PageHandleBase :: getBytes () {
    TimeStamp--; // read operation, need a new timestamp
    page->getBufferManager()->getLRU()->update(page); // update LRU because of new timestamp
    return page->getRawData();
}

void MyDB_PageHandleBase :: wroteBytes () {
    page->setDirty(true);
    TimeStamp--; // write operation
    page->getBufferManager()->getLRU()->update(page);
}

MyDB_PageHandleBase :: ~MyDB_PageHandleBase () {
    page->removeReference(); // delete one page handle for one page
}

MyDB_PageHandleBase :: MyDB_PageHandleBase (MyDB_PagePtr page) {
    this->page = page;
    this->page->addReference();
}

MyDB_PagePtr MyDB_PageHandleBase :: getPage() {
    return page;
}

#endif