#ifndef MEM_H
#define MEM_H

#include <string.h>
#include <stddef.h>

/**
 * Which memory allocator should you use?
 *
 *   [Bound on number of items]  --no->   [Same size items?]  --no->   [Out of order free?]  --yes->   *malloc*
 *                | yes                          | yes                         | no
 *                |                              v                             v
 *                |                       *Group of Block*              *Group of stack*
 *                |
 *        [Same size items]  --no->  *Stack*
 *                | yes
 *                v
 *             *Block*                   
 */

#define mem__alignof(type) offsetof(struct {char a; type b;}, b)

#ifdef MEM_NO_STATIC
  #define MEM__CALL
#else
  #define MEM__CALL static
#endif


/**
 * Stack allocator
 *
 * A static-size stack. Supports push and pops
 *
 * order-constraint: stack
 * ptr-validity: yes
 */

typedef struct {
  unsigned char *begin, *end, *curr;
} Stack;

MEM__CALL void stack_init(Stack *stack, void *mem, long size);
#define stack_clear(stack) ((stack)->curr = (stack)->begin)
#define stack_push(stack, type) stack_push_ex(stack, sizeof(type), mem__alignof(type))
#define stack_push_val(stack, ptr) stack__push_val(stack, sizeof(*ptr), mem__alignof(*ptr), ptr)
#define stack_pop(stack, to) ((stack)->curr = (unsigned char*)(to))


/**
 * Block allocator
 *
 * Divides a fixed-size memory area into fixed-size blocks
 *
 * order-constraint: none
 * ptr_validity: yes
 *
 * constraints:
 *   Because the next-pointers are stored in the unused items, the item size must be at least sizeof(void*),
 *   and the memory given is properly aligned for both the intended type, and void*
 */

typedef struct {
  unsigned char *next;
  long item_size;
} Block;

MEM__CALL int block_init(Block *block, void *mem, long num_items, long item_size);
MEM__CALL void* block_get(Block *block);
MEM__CALL void block_put(Block *block, void *at);


/**
 * errors
 */

extern int mem_errno;
enum {
  MEM_FULL = 1,
  MEM_INVALID_ALIGN,
  MEM_INVALID_ARG
};






/* PRIVATE HEADER */

MEM__CALL void* stack_push_ex(Stack *stack, long size, int align);
MEM__CALL void* stack__push_val(Stack *stack, long size, int align, void *ptr);
MEM__CALL void stack__pop(Stack *stack, long size, int align);


#endif /* MEM_H */

#ifdef MEM_IMPLEMENTATION

int mem_errno;

#define mem__align(x, val) (void*)(((long)(x)+((val)-1)) & ~((val)-1))

/* STACK IMPLEMENTATION */


MEM__CALL void stack_init(Stack *stack, void *mem, long size) {
  stack->begin = stack->curr = mem;
  stack->end = stack->begin + size;
}

MEM__CALL void* stack_push_ex(Stack *stack, long size, int align) {

  stack->curr = mem__align(stack->curr, align);
  if (stack->curr + size > stack->end) {
    mem_errno = MEM_FULL;
    return 0;
  }

  stack->curr += size;

  return stack->curr - size;
}

MEM__CALL void* stack__push_val(Stack *stack, long size, int align, void *ptr) {
  unsigned char *p;

  p = stack_push_ex(stack, size, align);
  if (!p)
    return 0;

  memcpy(p, ptr, size);
  return p;
}

/* BLOCK IMPLEMENTATION */

MEM__CALL int block_init(Block *block, void *mem, long num_items, long item_size) {
  int i;
  unsigned char *p;

  if (item_size < (long)sizeof(void*))
    return mem_errno = MEM_INVALID_ARG;

  if ((long)mem & (mem__alignof(void*)-1))
    return mem_errno = MEM_INVALID_ALIGN;

  block->item_size = item_size;

  /* set up chain pointers */
  for (p = (unsigned char*)mem, i = 0; i < num_items-1; ++i, p += item_size)
    *(void**)p = p+item_size;
  *(void**)p = 0;

  block->next = (unsigned char*)mem;

  return 0;
}

MEM__CALL void* block_get(Block *block) {
  void *p;

  p = block->next;
  if (!p) {
    mem_errno = MEM_FULL;
    return 0;
  }

  block->next = *(void**)p;
  return p;
}

MEM__CALL void block_put(Block *block, void *at) {
  *(void**)at = block->next;
  block->next = at;
}

#endif /* MEM_IMPLEMENTATION */