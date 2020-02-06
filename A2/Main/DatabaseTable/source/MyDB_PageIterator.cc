//
// Created by Dingjia Li on 2/2/20.
//

#include "MyDB_PageIterator.h"
#include "MyDB_PageReaderWriter.h"


MyDB_PageIterator :: MyDB_PageIterator(MyDB_RecordPtr recordPtr, MyDB_PageHandle pageHandle, MyDB_PageHeaderPtr headerPtr) {
    record = recordPtr;
    page = pageHandle;
    header = headerPtr;
    seenBytes = 0;
}

bool MyDB_PageIterator :: hasNext() {
    return seenBytes < header->getDataSize();
}


void MyDB_PageIterator :: getNext() {
    void * pos = header->getData() + seenBytes;
    void * newPos = record->fromBinary(pos);
    seenBytes += ((char *) newPos - (char *) pos);
}
