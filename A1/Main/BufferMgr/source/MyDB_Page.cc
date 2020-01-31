//
// Created by Dingjia Li on 1/24/20.
//

#ifndef PAGE_C
#define PAGE_C

#include "MyDB_BufferManager.h"
#include "MyDB_Page.h"
#include <fcntl.h>
#include <unistd.h>
using namespace std;


MyDB_Page ::  MyDB_Page(MyDB_TablePtr tablePtr, long id, size_t pageSize, string tempFile,
        MyDB_BufferManager *bufferMgr, void *data, bool isPin, bool isAnonymous) : bufferMgr(bufferMgr) {
    this->parentTable = tablePtr;
    this->id = id;
    this->pageSize = pageSize;
    this->tempFile = tempFile;
    this->rawData = data;
    this->isPinned = isPin;
    this->isAnonymous = isAnonymous;
    this->tablePair = make_pair(tablePtr, id);
    // initialize
    this->isDirty = false;
    this->numReferences = 0;
}

MyDB_Page :: ~MyDB_Page () {

}

void MyDB_Page :: setDirty(bool isDirty) {
    this->isDirty = isDirty;
}

bool MyDB_Page :: getDirty() {
    return isDirty;
}

void MyDB_Page :: pinPage() {
    isPinned = true;
}

void MyDB_Page :: unpinPage() {
    isPinned = false;
}

bool MyDB_Page :: getPin() {
    return isPinned;
}


int MyDB_Page :: getNumReferences() {
    return numReferences;
}

MyDB_TablePtr MyDB_Page :: getParentTable() {
    return parentTable;
}

pair<MyDB_TablePtr, int> MyDB_Page :: getTablePair() {
    return tablePair;
}

void MyDB_Page :: setRawData(void *newData) {
    memcpy(rawData, newData, pageSize); // deep copy
}

void *MyDB_Page :: getRawData() {
    return rawData;
}

void MyDB_Page :: addReference() {
    numReferences++;
}

void* MyDB_Page :: readDataFromFile(){
    int fd;
    if (parentTable == nullptr) {  // for anonymous
        fd = open(tempFile.c_str(), O_CREAT | O_RDWR | O_SYNC, 0666);
    }
    else {
        fd = open(parentTable->getStorageLoc().c_str(), O_CREAT | O_RDWR | O_SYNC, 0666);
    }
    lseek(fd, pageSize * id, SEEK_SET);  // set offset for each page
    read(fd, rawData, pageSize * sizeof(char));
    close(fd);
    return rawData;
}

void MyDB_Page :: writeBackToFile(){
    int fd;
    if (parentTable == nullptr) { // for anonymous
        fd = open(tempFile.c_str(), O_CREAT | O_RDWR | O_SYNC, 0666);
    }
    else {
        fd = open(parentTable->getStorageLoc().c_str(), O_CREAT | O_RDWR | O_SYNC, 0666);
    }
    lseek(fd, pageSize * id, SEEK_SET); // set offset for each page
    write(fd, rawData, pageSize * sizeof(char));
    close(fd);
    isDirty = false;
}


MyDB_BufferManager * MyDB_Page :: getBufferManager() {
    return bufferMgr;
}

void MyDB_Page :: removeReference() {
    numReferences--;
    if (numReferences == 0) { // no page handles any more
        if (isAnonymous) { // move anonymous page out of memory
            bufferMgr->killPage(make_shared<MyDB_Page>(*this));
        }
        else {
            isPinned = false; // unpin page
        }
    }
}


#endif