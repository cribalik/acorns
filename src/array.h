#ifndef ARRAY_H
#define ARRAY_H

/**
*               Example
*
*   Array(double) d = 0;
*
*   array_push(d, 0.3);
*   array_push(d, 7.0);
*
*   for (int i = 0; i < array_len(d); ++i)
*     printf("%f\n", d[i]);
*/

/* API */

#ifndef ARRAY_INITIAL_SIZE
  #define ARRAY_INITIAL_SIZE 4
#endif

#ifndef ARRAY_REALLOC
  #include <stdlib.h>
  #define ARRAY_REALLOC realloc
#endif

#ifndef ARRAY_FREE
  #include <stdlib.h>
  #define ARRAY_FREE free
#endif

/* For annotating array types, since they just look like pointers */
#define Array(type) type*

#define array_insert(a, i, x) (array_resize((a), array_len(a)+1), memmove((a)+(i)+1, (a)+(i), (array__n(a) - (i)) * sizeof(*(a))), (a)[i] = (x))
#define array_len(a) ((a) ? array__n(a) : 0)
#define array_len_get(a) (array__n(a))
#define array_push(a, val) ((!(a) || array__n(a) == array__c(a) ? (a)=array__grow(a, sizeof(*(a)), 1) : 0), (a)[array__n(a)++] = val)
#define array_push_n(a, n) ((!(a) || array__n(a)+(n) >= array__c(a) ? (a)=array__grow(a, sizeof(*(a)), (n)) : 0), array__n(a) += (n))
#define array_last(a) (!(a) ? 0 : (a)+array__n(a)-1)
#define array_free(a) ((a) ? ARRAY_FREE(&array__n(a)),0 : 0)
#define array_cap(a) ((a) ? array__c(a) : 0)
#define array_resize(a, n) ((n) > array_len(a) ? array_push_n(a, (n) - array_len(a)) : (array__n(a) = (n)))

/* Internals */
#define array__c(a) ((int*)(a))[-1]
#define array__n(a) ((int*)(a))[-2]
static void* array__grow(void* a, int size, int num) {
  /* TODO: ensure we are always power of 2 */
  int newc = a ? (num + array__n(a) > array__c(a)*2 ? num + array__n(a) : array__c(a)*2) : (num > ARRAY_INITIAL_SIZE ? num : ARRAY_INITIAL_SIZE);
  int n = a ? array__n(a) : 0;
  a = (int*)ARRAY_REALLOC(a ? &array__n(a) : 0, newc*size + 2*sizeof(int)) + 2;
  array__n(a) = n;
  array__c(a) = newc;
  return a;
}

#endif /* ARRAY_H */
