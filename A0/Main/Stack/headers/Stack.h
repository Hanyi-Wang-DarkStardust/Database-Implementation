
#ifndef STACK_H
#define STACK_H

#include <iostream>
using namespace std;

// this is the node class used to build up the LIFO stack
template <class Data>
class Node {

private:

	Data holdMe;
	Node *next;
	
public:
    /*****************************************/
    /** WHATEVER CODE YOU NEED TO ADD HERE!! */
    /*****************************************/

    // constructor
    Node(Data newData) {
        holdMe = newData;
        next = NULL;
    }

    Node(Data newData, Node *nextNode) {
        holdMe = newData;
        next = nextNode;
    }

    // Getter methods
    Data getData() {
        return holdMe;
    }

    Node * getNext() {
        return next;
    }
};

// a simple LIFO stack
template <class Data> 
class Stack {

	Node <Data> *head;

public:
	// destroys the stack
	~Stack() {
	    /* your code here */
	    Node<Data> *next = NULL;
	    // loop until the last node is deleted
	    while (head != NULL) {
	        next = head;
	        head = head->getNext();
	        delete next;
	    }
	}

	// creates an empty stack
	Stack() {
	    /* your code here */
	    head = NULL;
	}

	// adds pushMe to the top of the stack
	void push(Data n) {
	    /* your code here */
	    // create new node at the front
	    Node<Data> *newNode;
        newNode = new Node<Data>(n, head);  // NEED TO DELETE MANUALLY
	    head = newNode;
	}

	// return true if there are not any items in the stack
	bool isEmpty() {
	    /* replace with your code */
	    if (head == NULL) return true;
	    return false;
	}

	// pops the item on the top of the stack off, returning it...
	// if the stack is empty, the behavior is undefined
	Data pop() {
	    /* replace with your code */
	    // delete top node at the front
	    if (isEmpty()) {
	        return Data();  // return template datatype if already empty
	    }
	    Data popValue = head->getData();
	    Node<Data> *nodeToDel = head;
	    head = head->getNext();
	    delete nodeToDel;   // DELETE NODE
	    return popValue;
	}
};

#endif
