/******************************************************************
 cs2123p4_DS.c by Justin Mungal
 
 Machine Improvement Proposal - Data Structure Functions
 
 Purpose:
 
 This file contians the standard queue and linked-list routines 
 provided by Larry. As per our previous programs, they do not need 
 to be documented in the same way as the student-written functions.
 
 Returns:
 N/A
 ******************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "cs2123p4.h"

//begin queue functions
int removeQ(Queue queue, QElement *pFromQElement)
{
    NodeQ *p;
    // check for empty
    if (queue->pHead == NULL)
        return FALSE;
    p = queue->pHead;
    *pFromQElement = p->element;
    queue->pHead = p->pNext;
    // Removing the node could make the list empty.
    // See if we need to update pFoot, due to empty list
    if (queue->pHead == NULL)
        queue->pFoot = NULL;
    free(p);
    return TRUE;
}

void insertQ(Queue queue, QElement element)
{
    NodeQ *pNew;
    pNew = allocNodeQ(queue, element);
    // check for empty
    if (queue->pFoot == NULL)
    {
        queue->pFoot = pNew;
        queue->pHead = pNew;
    }
    else
    {   // insert after foot
        queue->pFoot->pNext = pNew;
        queue->pFoot = pNew;
    }
}

NodeQ *allocNodeQ(Queue q, QElement value)
{
    NodeQ *pNew;
    pNew = (NodeQ *)malloc(sizeof(NodeQ));
    if (pNew == NULL)
        ErrExit(ERR_ALGORITHM, "No available memory for queue");
    pNew->element = value;
    pNew->pNext = NULL;
    return pNew;
}

Queue newQueue(char szQueueNm[])
{
    Queue q = (Queue)malloc(sizeof(QueueImp));
    // Mark the list as empty
    q->pHead = NULL;   // empty list
    q->pFoot = NULL;   // empty list
    strcpy(q->szQName, szQueueNm);
    q->lQueueWaitSum = 0;
    q->lQueueWidgetTotalCount = 0;
    return q;
}
//end queue functions

//begin linked list functions
int removeLL(LinkedList list, Event *pValue)
{
    NodeLL *p;
    
    if (list->pHead == NULL)
        return FALSE;
    
    *pValue = list->pHead->event;
    p = list->pHead;
    list->pHead = list->pHead->pNext;
    free(p);
    return TRUE;
}

NodeLL *insertOrderedLL(LinkedList list, Event value)
{
    NodeLL *pNew, *pPrecedes;
    
    // call searchLL to properly set our pPrecedes
    searchLL(list, value.iTime, &pPrecedes);
    
    // Allocate a node and insert.
    pNew = allocateNodeLL(list, value);
    
    // Check for inserting at the beginning of the list
    // this will also handle when the list is empty
    if (pPrecedes == NULL)
    {
        pNew->pNext = list->pHead;
        list->pHead = pNew;
    }
    else
    {
        pNew->pNext = pPrecedes->pNext;
        pPrecedes->pNext = pNew;
    }
    return pNew;
}

NodeLL *searchLL(LinkedList list, int match, NodeLL **ppPrecedes)
{
    NodeLL *p;
    
    // used when the list is empty or we need to insert at the beginning
    *ppPrecedes = NULL;
    
    // Traverse through the list looking for where the key belongs or
    // the end of the list.
    for (p = list->pHead; p != NULL; p = p->pNext)
    {
        if (match == p->event.iTime)
            return p;
        else if (match < p->event.iTime)
            return NULL;
        *ppPrecedes = p;
    }
    
    // Not found, return NULL
    return NULL;
}

LinkedList newLinkedList()
{
    LinkedList list = (LinkedList) malloc(sizeof(LinkedListImp));
    //Mark the list as empty
    list->pHead = NULL;   // empty list
    return list;
}

NodeLL *allocateNodeLL(LinkedList list, Event value)
{
    NodeLL *pNew;
    
    pNew = (NodeLL *)malloc(sizeof(NodeLL));
    
    if (pNew == NULL)
        ErrExit(ERR_ALGORITHM, "No available memory for linked list");
    
    pNew->event = value;
    pNew->pNext = NULL;
    return pNew;
}
//end queue functions