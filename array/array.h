#ifndef ARRAY_H
#define ARRAY_H

#define ARRAY_INITIAL_SIZE 4

#ifndef ARRAY_REALLOC
#include <stdlib.h>
#define ARRAY_REALLOC realloc
#endif

#ifndef ARRAY_FREE
#include <stdlib.h>
#define ARRAY_FREE free
#endif

/**
*               Example
*
*   double* d = 0;
*
*   array_push(d, 0.3);
*   array_push(d, 7.0);
*   array_push(d, 5);
*
*   // remove the second element
*   d[1] = d[array_len_get(d)--];
*   printf("number of items in array: %i", array_len(d));
*
*   printf("%d -- %d\n", d[0], d[1]); // 0.3 -- 5
*
*   char* msg = "hello";
*   char* str = 0;
*   memcpy(array_push_n(str, strlen(msg)+1), "hello")
*/

/* API */

#define array_len(a) ((a) ? array__n((a)) : 0)
#define array_len_get(a) (array__n(a))
#define array_push(a, val) ((!(a) || array__n(a) == array__c(a) ? (a)=array__grow(a, sizeof(*(a)), 1) : 0), a[array__n(a)++] = val)
#define array_push_n(a, n) ((!(a) || array__n(a)+(n) >= array__c(a) ? (a)=array__grow(a, sizeof(*(a)), n) : 0), array__n(a) += (n), (a)[array__n(a)-(n)])
#define array_free(a) ((a) ? ARRAY_FREE(&array__n(a)),0 : 0)

/* Internals */
#define array__c(a) ((int*)(a))[-1]
#define array__n(a) ((int*)(a))[-2]
static void* array__grow(void* a, int size, int num) {
  int newc = a ? (num + array__n(a) > array__c(a)*2 ? num + array__n(a) : array__c(a)*2) : (num > ARRAY_INITIAL_SIZE ? num : ARRAY_INITIAL_SIZE);
  int n = a ? array__n(a) : 0;
  a = (int*)ARRAY_REALLOC(a ? &array__n(a) : 0, newc*size + 2*sizeof(int)) + 2;
  array__n(a) = n;
  array__c(a) = newc;
  return a;
}

#endif /* ARRAY_H */
