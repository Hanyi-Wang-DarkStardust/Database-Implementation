
#ifndef TABLE_RW_C
#define TABLE_RW_C

#include <fstream>
#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableReaderWriter.h"

using namespace std;


MyDB_TableReaderWriter :: MyDB_TableReaderWriter (MyDB_TablePtr forMe, MyDB_BufferManagerPtr myBuffer) {
    table = forMe;
    bufferMgr = myBuffer;
    if (table->lastPage() == -1) {
        table->setLastPage(0);
        MyDB_PageHandle page = bufferMgr->getPage(table, table->lastPage());
        MyDB_PageReaderWriterPtr pageReaderWriter = make_shared<MyDB_PageReaderWriter>(page, bufferMgr);
        pageReaderWriter->clear();
        pageVec.push_back(pageReaderWriter);
    } else {
        for (int i = 0; i <= table->lastPage(); i++) {
            MyDB_PageHandle page = bufferMgr->getPage(table, i);
            MyDB_PageReaderWriterPtr pageReaderWriter = make_shared<MyDB_PageReaderWriter>(page, bufferMgr);
            pageVec.push_back(pageReaderWriter);
        }
    }
}



MyDB_PageReaderWriter MyDB_TableReaderWriter :: operator[] (size_t i) {
    // Check validity of i
    if (i < 0) {
        cout << "Invalid indexing" << endl;
        exit(EXIT_FAILURE);
    }
    while (i > table->lastPage()) {
        table->setLastPage(table->lastPage() + 1);
        MyDB_PageHandle page = bufferMgr->getPage(table, table->lastPage());
        MyDB_PageReaderWriterPtr targetPageReaderWriter = make_shared<MyDB_PageReaderWriter>(page, bufferMgr);
        targetPageReaderWriter->clear();
        pageVec.push_back(targetPageReaderWriter);
    }

    return *(pageVec[i]);
}


MyDB_RecordPtr MyDB_TableReaderWriter :: getEmptyRecord () {
    return make_shared<MyDB_Record>(table->getSchema());
}

MyDB_PageReaderWriter MyDB_TableReaderWriter :: last () {
    return *(pageVec.back());
}

void MyDB_TableReaderWriter :: append (MyDB_RecordPtr appendMe) {
    // Check if appending new rec to the last page is available (i.e. is the last page full?)
    bool appendSuccess = pageVec.back()->append(appendMe);
    if (!appendSuccess)
    {
        table->setLastPage(table->lastPage() + 1);
        MyDB_PageHandle page = bufferMgr->getPage(table, table->lastPage());
        MyDB_PageReaderWriterPtr lastPage = make_shared<MyDB_PageReaderWriter>(page, bufferMgr);
        lastPage->clear();
        lastPage->append(appendMe);
        pageVec.push_back(lastPage);
    }
}

MyDB_RecordIteratorPtr MyDB_TableReaderWriter :: getIterator (MyDB_RecordPtr iterateIntoMe) {
    return make_shared<MyDB_TableIterator>(iterateIntoMe, *this, table);
}

void MyDB_TableReaderWriter :: loadFromTextFile (string fromMe) {
    ifstream loadFile;
    loadFile.open(fromMe);
    if (loadFile.is_open()) {
        for (auto iter : pageVec) {
            iter->clear();
        }
        table->setLastPage(0);
        MyDB_RecordPtr emptyRecord = getEmptyRecord();
        string recordString;
        while (getline(loadFile,recordString)) {
            emptyRecord->fromString(recordString);
            append(emptyRecord);
        }
    } else {
        cout << "Cannot load from file!!!!" << endl;
        exit(EXIT_FAILURE);
    }
    loadFile.close();
}

void MyDB_TableReaderWriter :: writeIntoTextFile (string toMe) {
    ofstream writeFile;
    writeFile.open(toMe);
    if (writeFile.is_open()) {
        MyDB_RecordPtr emptyRecord = getEmptyRecord();
        MyDB_RecordIteratorPtr recordIter = getIterator(emptyRecord);
        while (recordIter->hasNext()) {
            recordIter->getNext();
            writeFile << emptyRecord << endl;   // have been reloaded
        }
    } else {
        cout << "Cannot Write File!!! " << endl;
        exit(EXIT_FAILURE);
    }
    writeFile.close();
}

#endif

