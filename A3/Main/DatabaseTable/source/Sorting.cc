
#ifndef SORT_C
#define SORT_C

#include <MyDB_PageListIteratorAlt.h>
#include "MyDB_PageReaderWriter.h"
#include "MyDB_TableRecIterator.h"
#include "MyDB_TableRecIteratorAlt.h"
#include "MyDB_TableReaderWriter.h"
#include "Sorting.h"

using namespace std;



//class comp{
//    MyDB_RecordPtr lhs;
//    MyDB_RecordPtr rhs;
//    function<bool()> comparator;
//
//public:
//
//    comp(function<bool()> Comparator, MyDB_RecordPtr RHS, MyDB_RecordPtr LHS) {
//        comparator = Comparator;
//        lhs = LHS;
//        rhs = RHS;
//    }
//
//    bool operator()(const MyDB_RecordIteratorAltPtr leftIter, const MyDB_RecordIteratorAltPtr rightIter) {
//        leftIter->getCurrent(lhs);
//        rightIter->getCurrent(rhs);
//        return !comparator();
//    }
//
//};
//
//void mergeIntoFile (MyDB_TableReaderWriter &sortIntoMe, vector <MyDB_RecordIteratorAltPtr> &mergeUs, function <bool ()> comparator,
//                    MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {
//    comp cmp(comparator, lhs, rhs);
//    priority_queue <MyDB_RecordIteratorAltPtr, vector <MyDB_RecordIteratorAltPtr>, comp> pq(cmp);
//    for (const auto& iter : mergeUs) {
//        pq.push(iter);
//    }
//
//    MyDB_RecordPtr container = make_shared<MyDB_Record>(sortIntoMe.getTable()->getSchema());
//    while(!pq.empty()) {
//        auto iter = pq.top();
//        pq.pop();
//        iter->getCurrent(container);
//        sortIntoMe.append(container);
//        if (iter->advance()) {
//            pq.push(iter);
//        }
//    }
//}



void mergeIntoFile(MyDB_TableReaderWriter &sortIntoMe,
                   vector<MyDB_RecordIteratorAltPtr> &mergeUs,
                   function<bool()> comparator,
                   MyDB_RecordPtr lhs,
                   MyDB_RecordPtr rhs) {
    auto cmp = [&lhs, &rhs, &comparator](const MyDB_RecordIteratorAltPtr &left,
                                         const MyDB_RecordIteratorAltPtr &right) {
        left->getCurrent(lhs);
        right->getCurrent(rhs);
        return !comparator(); //'>' from small to big, '<' from big to small
    };

    //declare the min heap
    priority_queue<MyDB_RecordIteratorAltPtr, vector<MyDB_RecordIteratorAltPtr>, decltype(cmp)> minPq(cmp);

    //push the first record of each page into the minPq
    for (MyDB_RecordIteratorAltPtr iterPtr: mergeUs) {
        if (iterPtr->advance()) {
            minPq.push(iterPtr);
        }
    }

    //create a record to store the next smaller record
    MyDB_RecordPtr cursorRecord = make_shared<MyDB_Record>(sortIntoMe.getTable()->getSchema());

    //update the minPq by taking the smallest record out of the minPq and insert the next bigger one
    while (!minPq.empty()) {
        MyDB_RecordIteratorAltPtr listIterPtr = minPq.top();
        minPq.pop();
        listIterPtr->getCurrent(cursorRecord);
        sortIntoMe.append(cursorRecord);
        if (listIterPtr->advance()) {
            minPq.push(listIterPtr);
        }
    }
}

// Append the recordPtr to the current page. If the page is full, add the full-page to mergedList and
// append the rec to a new page
void addRecToPage(MyDB_PageReaderWriter &page, MyDB_RecordPtr rec, vector<MyDB_PageReaderWriter> &mergedList,
            MyDB_BufferManagerPtr parent) {
    bool appendSuccess = page.append(rec);
    if (appendSuccess) {  // success!!
        return;
    } else {    // page is full, append didn't success
        mergedList.push_back(page); // put the full-page into mergedList
        MyDB_PageReaderWriter newPage(*parent); // create a new anonymous newPage
        page = newPage; // update the current page to be the new page
        page.append(rec);   //  Append rec to the current empty page
    }
}

vector<MyDB_PageReaderWriter> mergeIntoList(MyDB_BufferManagerPtr parent, MyDB_RecordIteratorAltPtr leftIter,
                                            MyDB_RecordIteratorAltPtr rightIter, function<bool()> comparator,
                                            MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {
    vector<MyDB_PageReaderWriter> mergedList;
    // create anonymous page
    MyDB_PageReaderWriter page(*parent);

    bool hasLeft = leftIter->advance();
    bool hasRight = rightIter->advance();

    if (hasLeft || hasRight) {
        // Both left and right still have records
        while (hasLeft && hasRight) {
            leftIter->getCurrent(lhs);
            rightIter->getCurrent(rhs);
            if (comparator()) { // left < right
                hasLeft = leftIter->advance();
                addRecToPage(page, lhs, mergedList, parent);
            } else {    // left >= right
                hasRight = rightIter->advance();
                addRecToPage(page, rhs, mergedList, parent);
            }
        }

        // Left still has, right done
        while (hasLeft) {
            leftIter->getCurrent(lhs);
            hasLeft = leftIter->advance();
            addRecToPage(page, lhs, mergedList, parent);
        }

        // Right still has, left done
        while (hasRight) {
            rightIter->getCurrent(rhs);
            hasRight = rightIter->advance();
            addRecToPage(page, rhs, mergedList, parent);
        }
    }
    // Now left and right finish merging
    mergedList.push_back(page);
    return mergedList;
}



void sort(int runSize, MyDB_TableReaderWriter &sortMe, MyDB_TableReaderWriter &sortIntoMe,
          function<bool()> comparator, MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {
    int numPages = sortMe.getNumPages();
    vector<MyDB_RecordIteratorAltPtr> mergedRec;
    deque<vector<MyDB_PageReaderWriter>> queueToMerge;

    for (int i = 0; i < numPages; i++) {
        // 1. Get current page and sort it
        MyDB_PageReaderWriter currPage = sortMe[i];
        MyDB_PageReaderWriter sortedPage = *currPage.sort(comparator, lhs, rhs);

        // 2. convert the current page into a vector(contain only that page)
        // and insert into the deque
        vector<MyDB_PageReaderWriter> pageVec;
        pageVec.push_back(sortedPage);
        queueToMerge.push_front(pageVec);

        // 3. If a run is full or reach the end, merge sort
        if (i == numPages - 1 || i % runSize == 0) {
            while (queueToMerge.size() > 1) {
                // pop twice
                vector<MyDB_PageReaderWriter> popPageVec1 = queueToMerge.back();
                queueToMerge.pop_back();

                vector<MyDB_PageReaderWriter> popPageVec2 = queueToMerge.back();
                queueToMerge.pop_back();

                // merge two vectors into one vector
                vector<MyDB_PageReaderWriter> mergedTwo;
                MyDB_RecordIteratorAltPtr popIter1 = getIteratorAlt(popPageVec1);
                MyDB_RecordIteratorAltPtr popIter2 = getIteratorAlt(popPageVec2);
                mergedTwo = mergeIntoList(sortMe.getBufferMgr(), popIter1, popIter2, comparator, lhs, rhs);

                // push into deque
                queueToMerge.push_front(mergedTwo);
            }
            // Merge sort is done
            vector<MyDB_PageReaderWriter> mergedPage = queueToMerge.back();
            mergedRec.push_back(getIteratorAlt(mergedPage));
            queueToMerge.pop_back();
        }
    }

    // Now we have mergedRec to be a vector of sorted run
    mergeIntoFile(sortIntoMe, mergedRec, comparator, lhs, rhs);
}

#endif
