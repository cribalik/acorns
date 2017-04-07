#ifndef ARRAY_H
#define ARRAY_H

/**
*               Example
*
*   double* d = array_create(double)
*
*   array_push(&d, 0.3);
*   array_push(&d, 7.0);
*   array_push(&d, 5);
*
*   printf("%d%d%d\n", d[0], d[1], d[2]);
*/

#define array_create(type) array_internal_create(sizeof(type))
int     array_len(void* a);
int     array_capacity(void* a);
#define array_push(arr_addr, val) array_internal_push((void**) arr_addr, sizeof(**arr_addr)), (*arr_addr)[array_len(a)-1] = val;




/* Internal API */
void  array_internal_push(void** arrp, int size);
void* array_internal_create(int size);

#endif

