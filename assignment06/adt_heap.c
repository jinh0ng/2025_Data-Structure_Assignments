#include <stdio.h>
#include <stdlib.h> // malloc, realloc, free

#include "adt_heap.h"

/* Reestablishes heap by moving data in child up to correct location heap array
   for heap_Insert function
*/
static void _reheapUp(HEAP *heap, int index);

/* Reestablishes heap by moving data in root down to its correct location in the heap
   for heap_Delete function
*/
static void _reheapDown(HEAP *heap, int index);

/* Reestablishes heap by moving data in child up to correct location in heap array
   for heap_Insert function
*/
static void _reheapUp(HEAP *heap, int index)
{
   int parent;
   void *tmp;

   while (index > 0)
   {
      parent = (index - 1) / 2;
      /* If child > parent (max-heap), swap */
      if (heap->compare(heap->heapArr[index], heap->heapArr[parent]) > 0)
      {
         tmp = heap->heapArr[parent];
         heap->heapArr[parent] = heap->heapArr[index];
         heap->heapArr[index] = tmp;
         index = parent;
      }
      else
      {
         break;
      }
   }
}

/* Reestablishes heap by moving data in root down to its correct location in the heap
   for heap_Delete function
*/
static void _reheapDown(HEAP *heap, int index)
{
   int left, right, larger;
   void *tmp;

   while (1)
   {
      left = 2 * index + 1;
      right = 2 * index + 2;

      /* No children */
      if (left >= heap->last)
      {
         break;
      }

      /* Determine which child is larger */
      if (right < heap->last)
      {
         /* Both children exist */
         if (heap->compare(heap->heapArr[left], heap->heapArr[right]) >= 0)
         {
            larger = left;
         }
         else
         {
            larger = right;
         }
      }
      else
      {
         /* Only left child exists */
         larger = left;
      }

      /* If child > parent, swap */
      if (heap->compare(heap->heapArr[larger], heap->heapArr[index]) > 0)
      {
         tmp = heap->heapArr[index];
         heap->heapArr[index] = heap->heapArr[larger];
         heap->heapArr[larger] = tmp;
         index = larger;
      }
      else
      {
         break;
      }
   }
}

/* Allocates memory for heap and returns address of heap head structure
   if memory overflow, NULL returned
   The initial capacity of the heap should be 10
*/
HEAP *heap_Create(int (*compare)(const void *arg1, const void *arg2))
{
   HEAP *heap = (HEAP *)malloc(sizeof(HEAP));
   if (heap == NULL)
   {
      return NULL;
   }
   heap->last = 0;
   heap->capacity = 10;
   heap->compare = compare;
   heap->heapArr = (void **)malloc(heap->capacity * sizeof(void *));
   if (heap->heapArr == NULL)
   {
      free(heap);
      return NULL;
   }
   return heap;
}

/* Free memory for heap */
void heap_Destroy(HEAP *heap, void (*remove_data)(void *ptr))
{
   int i;
   if (heap == NULL)
      return;
   /* Free each data element using provided function */
   for (i = 0; i < heap->last; i++)
   {
      remove_data(heap->heapArr[i]);
   }
   free(heap->heapArr);
   free(heap);
}

/* Inserts data into heap
   return 1 if successful; 0 if memory allocation failure
*/
int heap_Insert(HEAP *heap, void *dataPtr)
{
   void **newArr;
   if (heap == NULL || dataPtr == NULL)
      return 0;

   /* If capacity reached, double the array */
   if (heap->last >= heap->capacity)
   {
      newArr = (void **)realloc(heap->heapArr, (heap->capacity * 2) * sizeof(void *));
      if (newArr == NULL)
      {
         return 0;
      }
      heap->heapArr = newArr;
      heap->capacity *= 2;
   }

   /* Insert at the end and reheap up */
   heap->heapArr[heap->last] = dataPtr;
   _reheapUp(heap, heap->last);
   heap->last++;
   return 1;
}

/* Deletes root of heap and passes data back to caller
   return 1 if successful; 0 if heap empty
*/
int heap_Delete(HEAP *heap, void **dataOutPtr)
{
   if (heap == NULL || heap->last == 0)
   {
      return 0;
   }

   /* Pass root to caller */
   *dataOutPtr = heap->heapArr[0];

   /* Move last element to root, shrink, and reheap down */
   heap->heapArr[0] = heap->heapArr[heap->last - 1];
   heap->last--;
   _reheapDown(heap, 0);

   return 1;
}

/* return 1 if heap empty; 0 if not */
int heap_Empty(HEAP *heap)
{
   if (heap == NULL)
      return 1;
   return (heap->last == 0) ? 1 : 0;
}

/* Print heap array */
void heap_Print(HEAP *heap, void (*print_func)(const void *data))
{
   int i;
   if (heap == NULL)
   {
      printf("\n");
      return;
   }
   for (i = 0; i < heap->last; i++)
   {
      print_func(heap->heapArr[i]);
   }
   printf("\n");
}
