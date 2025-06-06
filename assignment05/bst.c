#include <stdlib.h>
#include <stdio.h>

#include "bst.h"

/* Internal helper prototypes */
static int _insert(NODE *root, NODE *newPtr,
                   int (*compare)(const void *, const void *),
                   void (*callback)(void *));
static NODE *_makeNode(void *dataInPtr);
static void _destroy(NODE *root, void (*destroy)(void *));
static NODE *_delete(NODE *root, void *keyPtr, void **dataOutPtr,
                     int (*compare)(const void *, const void *));
static NODE *_search(NODE *root, void *keyPtr,
                     int (*compare)(const void *, const void *));
static void _traverse(NODE *root, void (*callback)(const void *));
static void _traverseR(NODE *root, void (*callback)(const void *));
static void _inorder_print(NODE *root, int level, void (*callback)(const void *));

/* Create a new empty BST */
TREE *BST_Create(int (*compare)(const void *, const void *))
{
    TREE *pTree = (TREE *)malloc(sizeof(TREE));
    if (pTree)
    {
        pTree->root = NULL;
        pTree->compare = compare;
        pTree->count = 0;
    }
    return pTree;
}

/* Destroy the BST, freeing all nodes and their data */
void BST_Destroy(TREE *pTree, void (*destroy)(void *))
{
    if (pTree)
    {
        _destroy(pTree->root, destroy);
        free(pTree);
    }
}

/* Insert data into the BST. callback handles duplicates */
int BST_Insert(TREE *pTree, void *dataInPtr, void (*callback)(void *))
{
    NODE *newPtr = _makeNode(dataInPtr);
    if (!newPtr)
        return 0; /* malloc failed */

    if (pTree->root == NULL)
    {
        pTree->root = newPtr;
        pTree->count++;
        return 1;
    }

    int status = _insert(pTree->root, newPtr, pTree->compare, callback);
    if (status == 1)
    {
        pTree->count++;
        return 1;
    }
    /* status == -1 (duplicate) or 0 (error) */
    return status == -1 ? 1 : 0;
}

/* Delete a node matching keyPtr */
void *BST_Delete(TREE *pTree, void *keyPtr)
{
    void *dataOut = NULL;
    pTree->root = _delete(pTree->root, keyPtr, &dataOut, pTree->compare);
    if (dataOut)
        +pTree->count--;
    return dataOut;
}

/* Search for a node matching keyPtr */
void *BST_Search(TREE *pTree, void *keyPtr)
{
    NODE *found = _search(pTree->root, keyPtr, pTree->compare);
    return found ? found->dataPtr : NULL;
}
/* In-order traversal */
void BST_Traverse(TREE *pTree, void (*callback)(const void *))
{
    _traverse(pTree->root, callback);
}

/* Reverse in-order traversal */
void BST_TraverseR(TREE *pTree, void (*callback)(const void *))
{
    _traverseR(pTree->root, callback);
}

/* Print tree sideways */
void printTree(TREE *pTree, void (*callback)(const void *))
{
    _inorder_print(pTree->root, 0, callback);
}

/*----- Internal helper definitions -----*/

// static int _insert(NODE *root, NODE *newPtr,
//                    int (*compare)(const void *, const void *),
//                    void (*callback)(void *))
// {
//     int comp = compare(newPtr->dataPtr, root->dataPtr);
//     if (comp == 0)
//     {
//         if (callback)
//             callback(root->dataPtr);
//         free(newPtr);
//         return -1;
//     }
//     else if (comp < 0)
//     {
//         if (root->left)
//             return _insert(root->left, newPtr, compare, callback);
//         root->left = newPtr;
//         return 1;
//     }
//     else
//     {
//         if (root->right)
//             return _insert(root->right, newPtr, compare, callback);
//         root->right = newPtr;
//         return 1;
//     }
// }

static int _insert(NODE *root, NODE *newPtr,
                   int (*compare)(const void *, const void *),
                   void (*callback)(void *))
{
    int comp = compare(newPtr->dataPtr, root->dataPtr);

    /* 중복된 단어: 빈도만 증가시키고, 새로 할당된 payload와 노드는 전부 해제 */
    if (comp == 0)
    {
        if (callback)
            callback(root->dataPtr);

        /* newPtr->dataPtr가 가리키는 구조체의 첫 번째 멤버(char *word) 꺼내기 */
        char *dupWord = *(char **)(newPtr->dataPtr);
        free(dupWord);         /* strdup으로 할당된 문자열 해제 */
        free(newPtr->dataPtr); /* tWord 구조체 자체 해제 */
        free(newPtr);          /* NODE 구조체 해제 */

        return -1;
    }

    if (comp < 0)
    {
        if (root->left)
            return _insert(root->left, newPtr, compare, callback);
        root->left = newPtr;
        return 1;
    }
    else
    {
        if (root->right)
            return _insert(root->right, newPtr, compare, callback);
        root->right = newPtr;
        return 1;
    }
}

static NODE *_makeNode(void *dataInPtr)
{
    NODE *newNode = (NODE *)malloc(sizeof(NODE));
    if (!newNode)
        return NULL;
    newNode->dataPtr = dataInPtr;
    newNode->left = newNode->right = NULL;
    return newNode;
}

static void _destroy(NODE *root, void (*destroy)(void *))
{
    if (root)
    {
        _destroy(root->left, destroy);
        _destroy(root->right, destroy);
        if (destroy)
            destroy(root->dataPtr);
        free(root);
    }
}

static NODE *_delete(NODE *root, void *keyPtr, void **dataOutPtr,
                     int (*compare)(const void *, const void *))
{
    if (!root)
        return NULL;

    int comp = compare(keyPtr, root->dataPtr);
    if (comp < 0)
    {
        root->left = _delete(root->left, keyPtr, dataOutPtr, compare);
        return root;
    }
    if (comp > 0)
    {
        root->right = _delete(root->right, keyPtr, dataOutPtr, compare);
        return root;
    }

    /* 삭제 대상 노드(root) 발견 */
    /* 자식이 두 개인 경우: in-order successor로 대체 */
    if (root->left && root->right)
    {
        /* 1) 삭제할 원본 데이터 보관 */
        if (dataOutPtr)
            *dataOutPtr = root->dataPtr;
        /* 2) 오른쪽 서브트리에서 가장 작은 노드 찾기 */
        NODE *succParent = root;
        NODE *succ = root->right;
        while (succ->left)
        {
            succParent = succ;
            succ = succ->left;
        }

        /* 3) successor 데이터만 복사 */
        root->dataPtr = succ->dataPtr;

        /* 4) successor 노드 삭제 (dataOutPtr 덮어쓰지 않도록 NULL 전달) */
        if (succParent->left == succ)
            succParent->left = _delete(succ, succ->dataPtr, NULL, compare);
        else
            succParent->right = _delete(succ, succ->dataPtr, NULL, compare);

        return root;
    }

    /* 자식이 하나 이하인 경우: 바로 제거 */
    //*dataOutPtr = root->dataPtr;
    if (dataOutPtr)
        *dataOutPtr = root->dataPtr;
    NODE *next = root->left ? root->left : root->right;
    free(root);
    return next;
}

static NODE *_search(NODE *root, void *keyPtr,
                     int (*compare)(const void *, const void *))
{
    if (!root)
        return NULL;
    int comp = compare(keyPtr, root->dataPtr);
    if (comp == 0)
        return root;
    else if (comp < 0)
        return _search(root->left, keyPtr, compare);
    else
        return _search(root->right, keyPtr, compare);
}

static void _traverse(NODE *root, void (*callback)(const void *))
{
    if (!root)
        return;
    _traverse(root->left, callback);
    callback(root->dataPtr);
    _traverse(root->right, callback);
}

static void _traverseR(NODE *root, void (*callback)(const void *))
{
    if (!root)
        return;
    _traverseR(root->right, callback);
    callback(root->dataPtr);
    _traverseR(root->left, callback);
}

static void _inorder_print(NODE *root, int level, void (*callback)(const void *))
{
    if (!root)
        return;
    _inorder_print(root->right, level + 1, callback);
    for (int i = 0; i < level; i++)
        printf("    ");
    callback(root->dataPtr);
    printf("\n");
    _inorder_print(root->left, level + 1, callback);
}

/* 반환: 트리 내 노드 수 */
int BST_Count(TREE *pTree)
{
    return pTree->count;
}
