#include <stdlib.h> // malloc

#include "adt_dlist.h"

// internal insert function
// inserts data into list
// for addNode function
// return	1 if successful
// 			0 if memory overflow
static int _insert(LIST *pList, NODE *pPre, void *dataInPtr)
{
    NODE *newNode = (NODE *)malloc(sizeof(NODE));
    if (!newNode)
        return 0;
    newNode->dataPtr = dataInPtr;
    if (!pPre)
    {
        // insert at head
        newNode->llink = NULL;
        newNode->rlink = pList->head;
        if (pList->head)
            pList->head->llink = newNode;
        pList->head = newNode;
        if (!pList->rear)
            pList->rear = newNode;
    }
    else
    {
        newNode->rlink = pPre->rlink;
        newNode->llink = pPre;
        if (pPre->rlink)
            pPre->rlink->llink = newNode;
        pPre->rlink = newNode;
        if (pList->rear == pPre)
            pList->rear = newNode;
    }
    pList->count++;
    return 1;
}

// internal delete function
// deletes data from list and saves the (deleted) data to dataOutPtr
// for removeNode function
static void _delete(LIST *pList, NODE *pPre, NODE *pLoc, void **dataOutPtr)
{
    if (!pLoc)
        return;
    if (!pPre)
    {
        pList->head = pLoc->rlink;
        if (pList->head)
            pList->head->llink = NULL;
    }
    else
    {
        pPre->rlink = pLoc->rlink;
        if (pLoc->rlink)
            pLoc->rlink->llink = pPre;
    }
    if (pList->rear == pLoc)
        pList->rear = pPre;
    *dataOutPtr = pLoc->dataPtr;
    free(pLoc);
    pList->count--;
}

LIST *createList(int (*compare)(const void *, const void *))
{
    LIST *list = (LIST *)malloc(sizeof(LIST));
    if (!list)
        return NULL;
    list->count = 0;
    list->head = list->rear = NULL;
    list->compare = compare;
    return list;
}

// internal search function
// searches list and passes back address of node containing target and its logical predecessor
// for addNode, removeNode, searchNode functions
// return	1 found
// 			0 not found
static int _search(LIST *pList, NODE **pPre, NODE **pLoc, void *pArgu)
{
    NODE *pre = NULL;
    NODE *loc = pList->head;
    while (loc)
    {
        int cmp = pList->compare(pArgu, loc->dataPtr);
        if (cmp == 0)
            break;
        if (cmp < 0)
            break;
        pre = loc;
        loc = loc->rlink;
    }
    *pPre = pre;
    *pLoc = loc;
    return (loc && pList->compare(pArgu, loc->dataPtr) == 0) ? 1 : 0;
}

void destroyList(LIST *pList, void (*callback)(void *))
{
    if (!pList)
        return;
    NODE *curr = pList->head;
    while (curr)
    {
        NODE *next = curr->rlink;
        if (callback)
            callback(curr->dataPtr);
        free(curr);
        curr = next;
    }
    free(pList);
}

int addNode(LIST *pList, void *dataInPtr, void (*callback)(const void *))
{
    if (!pList || !dataInPtr)
        return 0;
    NODE *pre = NULL, *loc = NULL;
    if (_search(pList, &pre, &loc, dataInPtr))
    {
        // duplicate
        if (callback)
            callback(loc->dataPtr);
        return 2;
    }
    // insert
    return _insert(pList, pre, dataInPtr);
}

int removeNode(LIST *pList, void *keyPtr, void **dataOutPtr)
{
    if (!pList)
        return 0;
    NODE *pre = NULL, *loc = NULL;
    if (!_search(pList, &pre, &loc, keyPtr))
        return 0;
    _delete(pList, pre, loc, dataOutPtr);
    return 1;
}

int searchNode(LIST *pList, void *pArgu, void **dataOutPtr)
{
    if (!pList)
        return 0;
    NODE *pre = NULL, *loc = NULL;
    if (_search(pList, &pre, &loc, pArgu))
    {
        if (dataOutPtr)
            *dataOutPtr = loc->dataPtr;
        return 1;
    }
    return 0;
}

int countList(LIST *pList)
{
    return pList ? pList->count : 0;
}

int emptyList(LIST *pList)
{
    return (pList && pList->count == 0) ? 1 : 0;
}

void traverseList(LIST *pList, void (*callback)(const void *))
{
    NODE *curr = pList->head;
    while (curr)
    {
        callback(curr->dataPtr);
        curr = curr->rlink;
    }
}

void traverseListR(LIST *pList, void (*callback)(const void *))
{
    NODE *curr = pList->rear;
    while (curr)
    {
        callback(curr->dataPtr);
        curr = curr->llink;
    }
}