
#ifndef BPLUS_C
#define BPLUS_C

#include <MyDB_PageListIteratorAlt.h>
#include "MyDB_INRecord.h"
#include "MyDB_BPlusTreeReaderWriter.h"
#include "MyDB_PageReaderWriter.h"
#include "MyDB_PageListIteratorSelfSortingAlt.h"
#include "RecordComparator.h"
#include "algorithm"

//TO DO
MyDB_BPlusTreeReaderWriter :: MyDB_BPlusTreeReaderWriter(string orderOnAttName, MyDB_TablePtr forMe,
                                                       MyDB_BufferManagerPtr myBuffer) : MyDB_TableReaderWriter(forMe, myBuffer) {

    // find the ordering attribute
    auto res = forMe->getSchema()->getAttByName(orderOnAttName);

    // remember information about the ordering attribute
    orderingAttType = res.second;   //MyDB_AttTypePtr
    whichAttIsOrdering = res.first; //int

    // and the root location
    rootLocation = getTable()->getRootLocation();

}


MyDB_RecordIteratorAltPtr MyDB_BPlusTreeReaderWriter :: getRangeIterHelper(vector<MyDB_PageReaderWriter> &list, MyDB_AttValPtr low, MyDB_AttValPtr high, bool needSort) {
    // Prepare other parameters for MyDB_PageListIteratorSelfSortingAlt
    MyDB_RecordPtr leftRec;
    MyDB_RecordPtr rightRec;
    function<bool()> comparator;

    MyDB_RecordPtr currRec;
    function<bool()> lowVSCurrComparator;
    function<bool()> CurrVSHighComparator;

    currRec = getEmptyRecord();
    // low Comparator: curr vs low
    MyDB_INRecordPtr lowRec = getINRecord();
    lowRec->setKey(low);
    lowVSCurrComparator = buildComparator(currRec, lowRec);

    // high Comparator: high vs curr
    MyDB_INRecordPtr highRec = getINRecord();
    highRec->setKey(high);
    CurrVSHighComparator = buildComparator(highRec, currRec);

    // Comparator: left vs right
    leftRec = getEmptyRecord();
    rightRec = getEmptyRecord();
    comparator = buildComparator(leftRec, rightRec);

    // Create Record Iterator over target pages
    MyDB_RecordIteratorAltPtr rangeIter =
            make_shared<MyDB_PageListIteratorSelfSortingAlt>(list, leftRec, rightRec, comparator, currRec,
                                                             lowVSCurrComparator, CurrVSHighComparator, needSort);
    return rangeIter;
}


MyDB_RecordIteratorAltPtr
MyDB_BPlusTreeReaderWriter :: getSortedRangeIteratorAlt(MyDB_AttValPtr low, MyDB_AttValPtr high) {
    // Get the pages within range
    vector<MyDB_PageReaderWriter> list;
    discoverPages(rootLocation, list, low, high);

    return getRangeIterHelper(list, low, high, true);
}


//TO DO
MyDB_RecordIteratorAltPtr MyDB_BPlusTreeReaderWriter :: getRangeIteratorAlt(MyDB_AttValPtr low, MyDB_AttValPtr high) {
    // Get the pages within range
    vector<MyDB_PageReaderWriter> list;
    discoverPages(rootLocation, list, low, high);
//    MyDB_RecordIteratorAltPtr rangeIter = make_shared<MyDB_PageListIteratorAlt>(list);
//    return rangeIter;

    return getRangeIterHelper(list, low, high, false);
}

bool MyDB_BPlusTreeReaderWriter :: discoverPages(int whichPage, vector<MyDB_PageReaderWriter> &list, MyDB_AttValPtr low, MyDB_AttValPtr high) {
    // Find the page
    MyDB_PageReaderWriter targetPage(*this, whichPage);
    queue<MyDB_PageReaderWriter> pageQ;
    pageQ.push(targetPage);

    MyDB_INRecordPtr lowRec = getINRecord();
    lowRec->setKey(low);
    MyDB_INRecordPtr highRec = getINRecord();
    highRec->setKey(high);

    while (!pageQ.empty()) {
        MyDB_PageReaderWriter popPage = pageQ.front();
        pageQ.pop();
        if (popPage.getType() == RegularPage) {
            list.push_back(popPage);
        } else {
            MyDB_INRecordPtr rec = getINRecord();
            MyDB_RecordIteratorAltPtr recIter = popPage.getIteratorAlt();
            function<bool()> lowComparator = buildComparator(rec, lowRec);
            function<bool()> highComparator = buildComparator(highRec, rec);
            while (recIter->advance()) {
                recIter->getCurrent(rec);
                bool lowInbound = !lowComparator();
                if (!lowInbound) continue;
                MyDB_PageReaderWriter nextPage = (*this)[rec->getPtr()];
                pageQ.push(nextPage);
                bool highInbound = !highComparator();
                if (!highInbound) break;
            }
        }
    }
    return false;
}


//TO DO
void MyDB_BPlusTreeReaderWriter :: append(MyDB_RecordPtr appendMe) {
    if (getNumPages() <= 1) { // no corresponding B+-Tree
        rootLocation = 0;
        getTable()->setRootLocation(0);
        getTable()->setLastPage(1);
        auto rootNode = (*this)[0];
        auto leafNode = (*this)[1];
        rootNode.clear();
        rootNode.setType(DirectoryPage);
        leafNode.clear();
        leafNode.setType(RegularPage);
        auto INF = getINRecord();
        INF->setPtr(1);
        rootNode.append(INF);
        leafNode.append (appendMe);
    }
    else {
        auto splitPtr = append(rootLocation, appendMe);
        if (splitPtr != nullptr) {
            int newRootLocation = getTable()->lastPage() + 1;
            getTable()->setLastPage(newRootLocation);
            getTable()->setRootLocation(newRootLocation);

            auto newRootNode = (*this)[newRootLocation];
            newRootNode.clear();
            newRootNode.setType(DirectoryPage);
            newRootNode.append(splitPtr);

            auto INF = getINRecord();
            INF->setPtr(rootLocation);
            newRootNode.append(INF);

            rootLocation = newRootLocation;
        } else return ;
    }
}


//TO DO
MyDB_RecordPtr MyDB_BPlusTreeReaderWriter :: split(MyDB_PageReaderWriter splitMe, MyDB_RecordPtr andMe) {
    // Node split:
    // Sort all records in over-fill page on the key
    // Choose median key (if even number, use upper one)
    // Create new pages
    // Kick a copy of median key up a level and adjust pointers

    bool isLeaf = splitMe.getType() == RegularPage;

    // 1. make an list containing records of over-filled page (splitMe + andMe)
    vector<MyDB_RecordPtr> over_filled_page;
    MyDB_RecordIteratorAltPtr iter = splitMe.getIteratorAlt();
    while (iter->advance()) {
        MyDB_RecordPtr rec = isLeaf? getEmptyRecord(): getINRecord();   // set the base class to be RecPtr
        iter->getCurrent(rec);
        over_filled_page.push_back(rec);
    }
    over_filled_page.push_back(andMe);    // support polymorphism

    // 2. Sort the over-filled page
    stable_sort(over_filled_page.begin(), over_filled_page.end(),
                [this](const MyDB_RecordPtr& lhs, const MyDB_RecordPtr& rhs){
                    auto cmp = buildComparator(lhs, rhs);
                    return cmp();
                });

    // 3. select median in the over-filled list
    auto median = over_filled_page.size() % 2 == 0 ? over_filled_page.size() / 2 : over_filled_page.size() / 2 + 1;

    // 4. create a new page
    int lastPageNum = getTable()->lastPage() + 1;
    getTable()->setLastPage(lastPageNum);
    lastPage = make_shared<MyDB_PageReaderWriter>(*this, lastPageNum);
    lastPage->clear();
    if (isLeaf)
        lastPage->setType(RegularPage);
    else
        lastPage->setType(DirectoryPage);

    // 5. put the small records in the new page and large records in old page
    splitMe.clear();
    if (isLeaf)
        splitMe.setType(RegularPage);
    else
        splitMe.setType(DirectoryPage);
    for (int i = 0; i < over_filled_page.size(); i++) {
        if (i < median)
            lastPage->append(over_filled_page[i]);
        else
            splitMe.append(over_filled_page[i]);
    }

    // 6. return the pointer to the record in new page
    MyDB_INRecordPtr newRec = getINRecord();
    auto largestRecNewPage = over_filled_page[median - 1];   // get the largest record in the new page (lastPage)
    newRec->setKey(getKey(largestRecNewPage));
    newRec->setPtr(getTable()->lastPage());
    return newRec;
}

//TO DO
MyDB_RecordPtr MyDB_BPlusTreeReaderWriter::append(int whichPage, MyDB_RecordPtr appendMe) {
    MyDB_PageReaderWriter appendPageNode(*this, whichPage);
    if (appendPageNode.getType() == RegularPage) { // leaf node
        if (appendPageNode.append(appendMe)) {
            return nullptr; // no split
        }
        else {
            return split(appendPageNode, appendMe);
        }
    }
    else { // Internal node
        MyDB_INRecordPtr IN = getINRecord();
        function<bool()> comp = buildComparator(appendMe, IN);
        MyDB_RecordIteratorAltPtr recIter = appendPageNode.getIteratorAlt();
        do {
            recIter->getCurrent(IN);
            if (comp()) {  // < current IN
                int pageNodeId = IN->getPtr();
                MyDB_RecordPtr splitPtr = append(pageNodeId, appendMe); // recursively find a position
                if (splitPtr) {
                    bool pageAppendSuccess = appendPageNode.append(splitPtr);
                    if (!pageAppendSuccess) return split(appendPageNode, splitPtr); // current node cannot insert more ptr
                    else {
                        MyDB_INRecordPtr compIN = getINRecord();
                        function<bool()> cmp = buildComparator(splitPtr, compIN);
                        appendPageNode.sortInPlace(cmp, splitPtr, compIN);
                        return nullptr;
                    }
                }
                else
                    return nullptr;
            } else continue;
        } while (recIter->advance());
    }
}


void MyDB_BPlusTreeReaderWriter :: printTree() {
    vector<vector<vector<MyDB_RecordPtr>>> level_order_list = getPrintTreeLevel(rootLocation);
    for (int i = 0; i < level_order_list.size(); i++) {
        vector<vector<MyDB_RecordPtr>> list = level_order_list[i];
        for (int j = 0; j < list.size(); j++) {
            vector<MyDB_RecordPtr> tmpList = list[j];
            for (int k = 0; k < tmpList.size(); k++) {
                cout << tmpList[k] << endl;
            }
            cout << endl;
        }
        cout << endl;
    }
}

vector<vector<vector<MyDB_RecordPtr>>> MyDB_BPlusTreeReaderWriter::getPrintTreeLevel(int root) {
    MyDB_PageReaderWriter rootPage = (*this)[root];
    vector<vector<vector<MyDB_RecordPtr>>> level_order_list;
    queue<MyDB_PageReaderWriter> pageQ;
    pageQ.push(rootPage);
    while (!pageQ.empty()) {
        int size = pageQ.size();
        vector<vector<MyDB_RecordPtr>> list;
        for (int i = 0; i < size; i++) {
            auto popPage = pageQ.front();
            pageQ.pop();
            vector<MyDB_RecordPtr> tmpList;
            if (popPage.getType() == RegularPage) {
                MyDB_RecordIteratorAltPtr recIter = popPage.getIteratorAlt();
                while (recIter->advance()) {
                    MyDB_RecordPtr rec = getEmptyRecord();
                    recIter->getCurrent(rec);
                    tmpList.push_back(rec);
                }
            } else {
                MyDB_RecordIteratorAltPtr recIter = popPage.getIteratorAlt();
                while (recIter->advance()) {
                    MyDB_INRecordPtr rec = getINRecord();
                    recIter->getCurrent(rec);
                    tmpList.push_back(rec);
                    MyDB_PageReaderWriter nextPage = (*this)[rec->getPtr()];
                    pageQ.push(nextPage);
                }
            }
            list.push_back(tmpList);
        }
        level_order_list.push_back(list);
    }
    return level_order_list;
}

//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------

MyDB_INRecordPtr MyDB_BPlusTreeReaderWriter :: getINRecord() {
    return make_shared<MyDB_INRecord>(orderingAttType->createAttMax());
}

MyDB_AttValPtr MyDB_BPlusTreeReaderWriter::getKey(MyDB_RecordPtr fromMe) {

    // in this case, got an IN record
    if (fromMe->getSchema() == nullptr)
        return fromMe->getAtt(0)->getCopy();

        // in this case, got a data record
    else
        return fromMe->getAtt(whichAttIsOrdering)->getCopy();
}

function<bool()> MyDB_BPlusTreeReaderWriter :: buildComparator(MyDB_RecordPtr lhs, MyDB_RecordPtr rhs) {

    MyDB_AttValPtr lhAtt, rhAtt;

    // in this case, the LHS is an IN record
    if (lhs->getSchema() == nullptr) {
        lhAtt = lhs->getAtt(0);

        // here, it is a regular data record
    } else {
        lhAtt = lhs->getAtt(whichAttIsOrdering);
    }

    // in this case, the LHS is an IN record
    if (rhs->getSchema() == nullptr) {
        rhAtt = rhs->getAtt(0);

        // here, it is a regular data record
    } else {
        rhAtt = rhs->getAtt(whichAttIsOrdering);
    }

    // now, build the comparison lambda and return
    if (orderingAttType->promotableToInt()) {
        return [lhAtt, rhAtt] { return lhAtt->toInt() < rhAtt->toInt(); };
    } else if (orderingAttType->promotableToDouble()) {
        return [lhAtt, rhAtt] { return lhAtt->toDouble() < rhAtt->toDouble(); };
    } else if (orderingAttType->promotableToString()) {
        return [lhAtt, rhAtt] { return lhAtt->toString() < rhAtt->toString(); };
    } else {
        cout << "This is bad... cannot do anything with the >.\n";
        exit(1);
    }
}



#endif