#include <stdio.h>
#include <stdlib.h> // malloc
#include <string.h> // strdup, strcmp
#include <ctype.h>	// toupper

#define QUIT 1
#define FORWARD_PRINT 2
#define BACKWARD_PRINT 3
#define SEARCH 4
#define DELETE 5
#define COUNT 6

// User structure type definition
// 단어 구조체
typedef struct
{
	char *word; // 단어
	int freq;	// 빈도
} tWord;

////////////////////////////////////////////////////////////////////////////////
// LIST type definition
typedef struct node
{
	tWord *dataPtr;
	struct node *llink; // backward pointer
	struct node *rlink; // forward pointer
} NODE;

typedef struct
{
	int count;
	NODE *head;
	NODE *rear;
} LIST;

////////////////////////////////////////////////////////////////////////////////
// Prototype declarations

// Allocates dynamic memory for a list head node and returns its address to caller
// return	head node pointer
// 			NULL if overflow
LIST *createList(void);

//  단어 리스트에 할당된 메모리를 해제 (head node, data node, word data)
void destroyList(LIST *pList);

// Inserts data into list
// return	0 if overflow
//			1 if successful
//			2 if duplicated key (이미 저장된 단어는 빈도 증가)
int addNode(LIST *pList, tWord *dataInPtr);

// Removes data from list
//	return	0 not found
//			1 deleted
int removeNode(LIST *pList, tWord *keyPtr, tWord **dataOutPtr);

// interface to search function
//	pArgu	key being sought
//	dataOutPtr	contains found data
//	return	1 successful
//			0 not found
int searchNode(LIST *pList, tWord *pArgu, tWord **dataOutPtr);

// returns number of nodes in list
int countList(LIST *pList);

// returns	1 empty
//			0 list has data
int emptyList(LIST *pList);

// traverses data from list (forward)
void traverseList(LIST *pList, void (*callback)(const tWord *));

// traverses data from list (backward)
void traverseListR(LIST *pList, void (*callback)(const tWord *));

// internal insert function
// inserts data into list
// for addNode function
// return	1 if successful
// 			0 if memory overflow
static int _insert(LIST *pList, NODE *pPre, tWord *dataInPtr);

// internal delete function
// deletes data from list and saves the (deleted) data to dataOutPtr
// for removeNode function
static void _delete(LIST *pList, NODE *pPre, NODE *pLoc, tWord **dataOutPtr);

// internal search function
// searches list and passes back address of node containing target and its logical predecessor
// for addNode, removeNode, searchNode functions
// return	1 found
// 			0 not found
static int _search(LIST *pList, NODE **pPre, NODE **pLoc, tWord *pArgu);

////////////////////////////////////////////////////////////////////////////////
// 단어 구조체를 위한 메모리를 할당하고 word, freq 초기화// return	word structure pointer
// return	할당된 단어 구조체에 대한 pointer
//			NULL if overflow
tWord *createWord(char *word);

//  단어 구조체에 할당된 메모리를 해제
// for destroyList function
void destroyWord(tWord *pNode);

////////////////////////////////////////////////////////////////////////////////
// gets user's input
int get_action()
{
	char ch;
	scanf("%c", &ch);
	ch = toupper(ch);
	switch (ch)
	{
	case 'Q':
		return QUIT;
	case 'P':
		return FORWARD_PRINT;
	case 'B':
		return BACKWARD_PRINT;
	case 'S':
		return SEARCH;
	case 'D':
		return DELETE;
	case 'C':
		return COUNT;
	}
	return 0; // undefined action
}

// compares two words in word structures
// for _search function
// 정렬 기준 : 단어
int compare_by_word(const void *n1, const void *n2)
{
	tWord *p1 = (tWord *)n1;
	tWord *p2 = (tWord *)n2;

	return strcmp(p1->word, p2->word);
}

// prints contents of word structure
// for traverseList and traverseListR functions
void print_word(const tWord *dataPtr)
{
	printf("%s\t%d\n", dataPtr->word, dataPtr->freq);
}

// gets user's input
void input_word(char *word)
{
	fprintf(stderr, "Input a word to find: ");
	fscanf(stdin, "%s", word);
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
	LIST *list;

	char word[100];
	tWord *pWord;
	int ret;
	FILE *fp;

	if (argc != 2)
	{
		fprintf(stderr, "usage: %s FILE\n", argv[0]);
		return 1;
	}

	fp = fopen(argv[1], "rt");
	if (!fp)
	{
		fprintf(stderr, "Error: cannot open file [%s]\n", argv[1]);
		return 2;
	}

	// creates an empty list
	list = createList();
	if (!list)
	{
		printf("Cannot create list\n");
		return 100;
	}

	while (fscanf(fp, "%s", word) != EOF)
	{
		pWord = createWord(word);

		// 이미 저장된 단어는 빈도 증가
		ret = addNode(list, pWord);

		if (ret == 0 || ret == 2) // failure or duplicated
		{
			destroyWord(pWord);
		}
	}

	fclose(fp);

	fprintf(stderr, "Select Q)uit, P)rint, B)ackward print, S)earch, D)elete, C)ount: ");

	while (1)
	{
		tWord *ptr;
		int action = get_action();

		switch (action)
		{
		case QUIT:
			destroyList(list);
			return 0;

		case FORWARD_PRINT:
			traverseList(list, print_word);
			break;

		case BACKWARD_PRINT:
			traverseListR(list, print_word);
			break;

		case SEARCH:
			input_word(word);

			pWord = createWord(word);

			if (searchNode(list, pWord, &ptr))
				print_word(ptr);
			else
				fprintf(stdout, "%s not found\n", word);

			destroyWord(pWord);
			break;

		case DELETE:
			input_word(word);

			pWord = createWord(word);

			if (removeNode(list, pWord, &ptr))
			{
				fprintf(stdout, "%s\t%d deleted\n", ptr->word, ptr->freq);
				destroyWord(ptr);
			}
			else
				fprintf(stdout, "%s not found\n", word);

			destroyWord(pWord);
			break;

		case COUNT:
			fprintf(stdout, "%d\n", countList(list));
			break;
		}

		if (action)
			fprintf(stderr, "Select Q)uit, P)rint, B)ackward print, S)earch, D)elete, C)ount: ");
	}
	return 0;
}

// 리스트 헤더 생성
LIST *createList(void)
{
	LIST *pList = (LIST *)malloc(sizeof(LIST));
	if (!pList)
		return NULL;
	pList->count = 0;
	pList->head = pList->rear = NULL;
	return pList;
}

// 리스트 및 모든 노드, 단어 메모리 해제
void destroyList(LIST *pList)
{
	NODE *p = pList->head;
	while (p)
	{
		NODE *next = p->rlink;
		destroyWord(p->dataPtr);
		free(p);
		p = next;
	}
	free(pList);
}

// 노드 추가 (중복 시 빈도 증가)
int addNode(LIST *pList, tWord *dataInPtr)
{
	NODE *pPre = NULL, *pLoc = NULL;
	if (_search(pList, &pPre, &pLoc, dataInPtr))
	{
		pLoc->dataPtr->freq++;
		return 2;
	}
	return _insert(pList, pPre, dataInPtr);
}

// 노드 삭제
int removeNode(LIST *pList, tWord *keyPtr, tWord **dataOutPtr)
{
	NODE *pPre = NULL, *pLoc = NULL;
	if (!_search(pList, &pPre, &pLoc, keyPtr))
		return 0;
	_delete(pList, pPre, pLoc, dataOutPtr);
	return 1;
}

// 노드 검색
int searchNode(LIST *pList, tWord *pArgu, tWord **dataOutPtr)
{
	NODE *pPre = NULL, *pLoc = NULL;
	if (_search(pList, &pPre, &pLoc, pArgu))
	{
		*dataOutPtr = pLoc->dataPtr;
		return 1;
	}
	return 0;
}

// 리스트 노드 수 반환
int countList(LIST *pList)
{
	return pList->count;
}

// 리스트 비어있는지 확인
int emptyList(LIST *pList)
{
	return (pList->count == 0);
}

// 순방향 순회
void traverseList(LIST *pList, void (*callback)(const tWord *))
{
	NODE *p = pList->head;
	while (p)
	{
		callback(p->dataPtr);
		p = p->rlink;
	}
}

// 역방향 순회
void traverseListR(LIST *pList, void (*callback)(const tWord *))
{
	NODE *p = pList->rear;
	while (p)
	{
		callback(p->dataPtr);
		p = p->llink;
	}
}

// 내부 삽입 함수
static int _insert(LIST *pList, NODE *pPre, tWord *dataInPtr)
{
	NODE *pNew = (NODE *)malloc(sizeof(NODE));
	if (!pNew)
		return 0;
	pNew->dataPtr = dataInPtr;
	if (pList->head == NULL)
	{
		pNew->llink = pNew->rlink = NULL;
		pList->head = pList->rear = pNew;
	}
	else if (pPre == NULL)
	{
		pNew->llink = NULL;
		pNew->rlink = pList->head;
		pList->head->llink = pNew;
		pList->head = pNew;
	}
	else if (pPre->rlink == NULL)
	{
		pNew->rlink = NULL;
		pNew->llink = pPre;
		pPre->rlink = pNew;
		pList->rear = pNew;
	}
	else
	{
		pNew->rlink = pPre->rlink;
		pNew->llink = pPre;
		pPre->rlink->llink = pNew;
		pPre->rlink = pNew;
	}
	pList->count++;
	return 1;
}

// 내부 삭제 함수
static void _delete(LIST *pList, NODE *pPre, NODE *pLoc, tWord **dataOutPtr)
{
	if (pPre == NULL)
	{
		pList->head = pLoc->rlink;
		if (pList->head)
			pList->head->llink = NULL;
		else
			pList->rear = NULL;
	}
	else
	{
		pPre->rlink = pLoc->rlink;
		if (pLoc->rlink)
			pLoc->rlink->llink = pPre;
		else
			pList->rear = pPre;
	}
	pList->count--;
	*dataOutPtr = pLoc->dataPtr;
	free(pLoc);
}

// 내부 검색 함수
static int _search(LIST *pList, NODE **pPre, NODE **pLoc, tWord *pArgu)
{
	*pPre = NULL;
	*pLoc = pList->head;
	while (*pLoc && compare_by_word((*pLoc)->dataPtr, pArgu) < 0)
	{
		*pPre = *pLoc;
		*pLoc = (*pLoc)->rlink;
	}
	if (*pLoc && compare_by_word((*pLoc)->dataPtr, pArgu) == 0)
		return 1;
	return 0;
}

// 단어 구조체 생성
tWord *createWord(char *word)
{
	tWord *p = (tWord *)malloc(sizeof(tWord));
	if (!p)
		return NULL;
	p->word = strdup(word);
	if (!p->word)
	{
		free(p);
		return NULL;
	}
	p->freq = 1;
	return p;
}

// 단어 구조체 메모리 해제
void destroyWord(tWord *pNode)
{
	if (pNode)
	{
		free(pNode->word);
		free(pNode);
	}
}
