//
// Created by Dingjia Li on 2/2/20.
//

#ifndef PAGE_ITERATOR_H
#define PAGE_ITERATOR_H

#include "MyDB_Record.h"
#include "MyDB_Page.h"
#include "MyDB_Table.h"
#include "MyDB_PageHandle.h"
#include "MyDB_PageHeader.h"
#include "MyDB_RecordIterator.h"


class MyDB_PageReaderWriter;

class MyDB_PageIterator : public MyDB_RecordIterator {
public:

    void getNext() override;

    bool hasNext() override;

    MyDB_PageIterator(MyDB_RecordPtr recordPtr, MyDB_PageHandle pageHandle, MyDB_PageHeaderPtr headerPtr);

private:
    MyDB_RecordPtr record;

    MyDB_PageHandle page;

    MyDB_PageHeaderPtr header;

    size_t seenBytes; // bytes of data that begins after dataSize (8 bytes) and pageType (4 bytes)

};


#endif //PAGE_ITERATOR_H
