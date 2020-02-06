//
// Created by Dingjia Li on 2/2/20.
//

#include "MyDB_TableIterator.h"
#include "MyDB_TableReaderWriter.h"


MyDB_TableIterator :: MyDB_TableIterator(MyDB_RecordPtr recordPtr, MyDB_TableReaderWriter& tableReaderWriter, MyDB_TablePtr tablePtr) : tableRW(tableReaderWriter) {
    record = recordPtr;
    table = tablePtr;
    curPage = 0;
    pageIter = tableRW[curPage].getIterator(record);
}



void MyDB_TableIterator :: getNext() {
    pageIter->getNext();
}

bool MyDB_TableIterator :: hasNext() {
    if (pageIter->hasNext()) { // current page still not full
        return true;
    }
    else {
        if (curPage == table->lastPage()) { // is already last page and is full
            return false;
        }
        else { // move to next page
            curPage++;
            pageIter = tableRW[curPage].getIterator(record);
            return hasNext();
        }
    }
}
