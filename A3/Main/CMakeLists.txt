cmake_minimum_required(VERSION 3.15)
project(Main)

set(CMAKE_CXX_STANDARD 14)

include_directories(BufferMgr/headers)
include_directories(Catalog/headers)
include_directories(DatabaseTable/headers)
include_directories(Qunit)
include_directories(Qunit/headers)
include_directories(Record/headers)

add_executable(Main
        BufferMgr/headers/CheckLRU.h
        BufferMgr/headers/MyDB_BufferManager.h
        BufferMgr/headers/MyDB_Page.h
        BufferMgr/headers/MyDB_PageHandle.h
        BufferMgr/headers/PageCompare.h
        BufferMgr/headers/TableCompare.h
        BufferMgr/source/MyDB_BufferManager.cc
        BufferMgr/source/MyDB_Page.cc
#        BufferTest/source/BufferQUnit.cc
        Catalog/headers/MyDB_AttType.h
        Catalog/headers/MyDB_Catalog.h
        Catalog/headers/MyDB_PageType.h
        Catalog/headers/MyDB_Schema.h
        Catalog/headers/MyDB_Table.h
        Catalog/source/MyDB_Catalog.cc
        Catalog/source/MyDB_Schema.cc
        Catalog/source/MyDB_Table.cc
        DatabaseTable/headers/MyDB_PageListIteratorAlt.h
        DatabaseTable/headers/MyDB_PageReaderWriter.h
        DatabaseTable/headers/MyDB_PageRecIterator.h
        DatabaseTable/headers/MyDB_PageRecIteratorAlt.h
        DatabaseTable/headers/MyDB_RecordIterator.h
        DatabaseTable/headers/MyDB_RecordIteratorAlt.h
        DatabaseTable/headers/MyDB_TableReaderWriter.h
        DatabaseTable/headers/MyDB_TableRecIterator.h
        DatabaseTable/headers/MyDB_TableRecIteratorAlt.h
        DatabaseTable/headers/RecordComparator.h
        DatabaseTable/headers/Sorting.h
        DatabaseTable/source/MyDB_PageListIteratorAlt.cc
        DatabaseTable/source/MyDB_PageReaderWriter.cc
        DatabaseTable/source/MyDB_PageRecIterator.cc
        DatabaseTable/source/MyDB_PageRecIteratorAlt.cc
        DatabaseTable/source/MyDB_TableReaderWriter.cc
        DatabaseTable/source/MyDB_TableRecIterator.cc
        DatabaseTable/source/MyDB_TableRecIteratorAlt.cc
        DatabaseTable/source/Sorting.cc
        Qunit/headers/QUnit.h
        Record/headers/MyDB_AttVal.h
        Record/headers/MyDB_INRecord.h
        Record/headers/MyDB_Record.h
        Record/source/MyDB_AttVal.cc
        Record/source/MyDB_Record.cc
#        RecordTest/source/RecordQUnit.cc
        SortTest/source/SortQUnit.cc)
