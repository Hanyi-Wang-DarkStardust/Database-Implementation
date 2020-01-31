//
// Created by Dingjia Li on 1/24/20.
//

#ifndef PAGE_H
#define PAGE_H

#include "MyDB_Table.h"
#include <memory>
#include <iostream>
#include <string.h>

using namespace std;

extern int TimeStamp;

class MyDB_BufferManager;

class MyDB_Page;
typedef shared_ptr <MyDB_Page> MyDB_PagePtr;
class MyDB_Page {

public:

    // page constructor
    MyDB_Page(MyDB_TablePtr tablePtr, long id, size_t pageSize, string tempFile, MyDB_BufferManager *bufferMgr, void *data, bool isPin, bool isAnonymous);

    ~MyDB_Page();

    void setDirty(bool isDirty);

    bool getDirty();

    void pinPage();

    void unpinPage();

    bool getPin();

    int getNumReferences();

    MyDB_TablePtr getParentTable();

    pair<MyDB_TablePtr, int> getTablePair();

    // set raw data, need to deep copy
    void setRawData(void * newData);

    void * getRawData();

    MyDB_BufferManager * getBufferManager();

    void writeBackToFile();

    void * readDataFromFile();

    void addReference(); // call when a handle created

    void removeReference(); // call when a handle deleted

private:
    bool isDirty;
    bool isPinned;
    bool isAnonymous;
    void *rawData;  // raw data in page
    long id;    // id of the page
    int numReferences;  // number of page handleS pointing to this page
    size_t pageSize;    // Size of page
    string tempFile;    // file for anonymous page to write on
    MyDB_TablePtr parentTable;  // table related to page, if the page is anonymous, this is nullptr
    MyDB_BufferManager *bufferMgr;  // pointer to buffer manager
    pair<MyDB_TablePtr, long> tablePair;    // unique identifier for this page
};


#endif //PAGE_H
