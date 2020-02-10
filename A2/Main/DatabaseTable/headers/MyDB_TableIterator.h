//
// Created by Dingjia Li on 2/2/20.
//

#ifndef TABLE_ITERATOR_H
#define TABLE_ITERATOR_H

#include "MyDB_Record.h"
#include "MyDB_RecordIterator.h"
#include "MyDB_PageIterator.h"

class MyDB_TableReaderWriter;

class MyDB_TableIterator : public MyDB_RecordIterator {
public:

    void getNext() override;

    bool hasNext() override;

    MyDB_TableIterator(MyDB_RecordPtr recordPtr, MyDB_TableReaderWriter& tableReaderWriter, MyDB_TablePtr tablePtr);

private:
    MyDB_RecordPtr record;
    MyDB_TableReaderWriter & tableRW; // convenient to use operator []
    size_t curPage;
    MyDB_TablePtr table;
    MyDB_RecordIteratorPtr pageIter;
};


#endif //TABLE_ITERATOR_H
