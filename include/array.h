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
*   d[1] = d[array_len(d)--];
*
*   printf("%d -- %d\n", d[0], d[1]); // 0.3 -- 5
*/

#define array_create(type) array_internal_create(sizeof(type))
#define array_len(arr) (*array_internal_len(arr))
#define array_push(arr_addr, val) array_internal_push((void**) arr_addr, sizeof(**arr_addr)), (*arr_addr)[array_len(*arr_addr)-1] = val
void    array_free(void* a);

/* Internal API */
int*  array_internal_len(void* arr);
void  array_internal_push(void** arrp, int size);
void* array_internal_create(int size);

#endif /* ARRAY_H */
