
#ifndef PAGE_RW_C
#define PAGE_RW_C

#include "MyDB_PageReaderWriter.h"
#include "MyDB_PageHeader.h"

MyDB_PageReaderWriter :: MyDB_PageReaderWriter(MyDB_PageHandle pageHandle, MyDB_BufferManagerPtr bufferManager) {
    page = pageHandle;
    bufferMgr = bufferManager;
    header = make_shared<MyDB_PageHeader>(page);
    header->setPageType(RegularPage);
}

void MyDB_PageReaderWriter :: clear () {
    header->setDataSize(0);
    header->setPageType(RegularPage);
    page->wroteBytes();
}

MyDB_PageType MyDB_PageReaderWriter :: getType () {
    return header->getPageType();
}

MyDB_RecordIteratorPtr MyDB_PageReaderWriter :: getIterator (MyDB_RecordPtr iterateIntoMe) {
	return make_shared<MyDB_PageIterator>(iterateIntoMe, page, header);
}

void MyDB_PageReaderWriter :: setType (MyDB_PageType pageType) {
    header->setPageType(RegularPage);
    page->wroteBytes();
}

bool MyDB_PageReaderWriter :: append (MyDB_RecordPtr appendMe) {
    char * next = header->getDataWithOffset() + appendMe->getBinarySize();
    if (next <= (char *) page->getBytes() + bufferMgr->getPageSize()) { // <= max address that a page can hold
        char * endPos = (char *) appendMe->toBinary(header->getDataWithOffset()); // will return next un-written byte
        header->setDataSize(endPos - header->getData());
        page->wroteBytes();
        return true;
    }
    else return false;
}


//MyDB_PageReaderWriter :: ~MyDB_PageReaderWriter() {
//
//}

#endif
