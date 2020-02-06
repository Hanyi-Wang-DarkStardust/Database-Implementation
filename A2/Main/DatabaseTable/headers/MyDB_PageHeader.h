//
// Created by Dingjia Li on 2/2/20.
//

#ifndef PAGE_PARSER_H
#define PAGE_PARSER_H

#include <iostream>
#include <memory>
#include "MyDB_PageHandle.h"
#include "MyDB_PageType.h"
using namespace std;

class MyDB_PageHeader;
typedef shared_ptr<MyDB_PageHeader> MyDB_PageHeaderPtr;

struct pageParser {
    size_t dataSize; // next un-written position
    MyDB_PageType pageType;
    char data[0]; // where data begins
};


class MyDB_PageHeader {
public:
    MyDB_PageHeader(MyDB_PageHandle pageHandle) {
        handle = pageHandle;
    }

    size_t getDataSize() {
        auto parser = (pageParser *) handle->getBytes();
        return parser->dataSize;
    }

    MyDB_PageType getPageType() {
        auto parser = (pageParser *) handle->getBytes();
        return parser->pageType;
    }

    char * getData() {
        auto parser = (pageParser *) handle->getBytes();
        return parser->data;
    }

    void setPageType(MyDB_PageType type) {
        auto parser = (pageParser *) handle->getBytes();
        parser->pageType = type;
    }

    void setDataSize(size_t dataSize) {
        auto parser = (pageParser *) handle->getBytes();
        parser->dataSize = dataSize;
    }

    char * getDataWithOffset() {
        auto parser = (pageParser *) handle->getBytes();
        return parser->data + parser->dataSize;
    }
private:
    MyDB_PageHandle handle;
};

#endif //PAGE_PARSER_H
