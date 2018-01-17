#ifndef MEM_H
#define MEM_H

#include <string.h>
#include <stddef.h>
#include <stdlib.h>

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

/* alignof */
#ifdef _MSC_VER
  #define mem__alignof __alignof
#else
  #define mem__alignof(type) offsetof(struct {char a; type b;}, b)
#endif

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

MEM__CALL int stack_init(Stack *stack, void *mem, long size);
#define stack_clear(stack) ((stack)->curr = (stack)->begin)
#define stack_push(stack, type) (stack_push_ex(stack, sizeof(type), mem__alignof(type)))
#define stack_push_val(stack, type, val) (stack__has_size(stack, type) ? (*(type*)stack_push(stack, type)=(val), (stack)->curr - sizeof(type)) : 0)
#define stack_pop(stack, to) ((stack)->curr = (unsigned char*)(to))







/**
 * LStack - a linked list of Stack allocators
 *
 * Supports the same operations and guarantees as a stack
 * Uses malloc for new stacks
 */

typedef struct {
  Stack stack;
  long block_size;
} LStack;

/* The size is the stack size, NOT the size of the blocks in the linked list.
 * If you for example want blocks of size 4096,
 * use lstack_size_from_blocksize when calling lstack_init */
MEM__CALL int lstack_init(LStack *ls, long size);
MEM__CALL int lstack__newblock(LStack *ls);
#define lstack_push(ls, type) (lstack_push_ex(ls, sizeof(type), mem__alignof(type)))
MEM__CALL void lstack_pop(LStack *ls, void *to);
MEM__CALL void lstack_clear(LStack *ls);

#define lstack_size_from_blocksize(size) ((size)-sizeof(Stack))







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
MEM__CALL int block_add_block(Block *block, void *mem, long num_items);
MEM__CALL void* block_get(Block *block);
MEM__CALL void block_put(Block *block, void *at);







/**
 * LBlock - a linked list of Block allocators
 *
 * Supports the same operations and guarantees as a block allocator
 * Uses malloc for new list items
 */

typedef struct {
  Block block;
  long num_items;
} LBlock;

/* The size is the stack size, NOT the size of the blocks in the linked list.
 * If you for example want blocks of size 4096,
 * use lblock_size_from_blocksize when calling lblock_init */
MEM__CALL int lblock_init(LBlock *lb, long num_items, long item_size);
MEM__CALL void* lblock_get(LBlock *lb);
MEM__CALL void lblock_put(LBlock *lb, void *at);

#define lblock_size_from_blocksize(size) ((size)-sizeof(Stack))






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

#define stack__has_size(stack, type) ((char*)mem__align((stack)->curr, mem__alignof(type)) + sizeof(type) <= (char*)(stack)->end)
MEM__CALL void* stack_push_ex(Stack *stack, long size, int align);
MEM__CALL void* stack__push_val(Stack *stack, long size, int align, void *ptr);
MEM__CALL void stack__pop(Stack *stack, long size, int align);

MEM__CALL void* lstack_push_ex(LStack *stack, long size, int align);
MEM__CALL int lstack_num_blocks(LStack *stack);

MEM__CALL void* lblock_push_ex(LBlock *lb, long size, int align);

#endif /* MEM_H */

#ifdef MEM_IMPLEMENTATION

int mem_errno;

#define mem__align(x, val) (void*)(((long)(x)+((val)-1)) & ~((val)-1))

/* STACK IMPLEMENTATION */

MEM__CALL int stack_init(Stack *stack, void *mem, long size) {
  if (!stack || !mem || !size)
    return mem_errno = MEM_INVALID_ARG;

  stack->begin = stack->curr = mem;
  stack->end = stack->begin + size;

  return 0;
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

/* LSTACK IMPLEMENTATION */

MEM__CALL int lstack_init(LStack *ls, long size) {
  ls->block_size = size + sizeof(Stack);

  memset(&ls->stack, 0, sizeof(ls->stack));
  return lstack__newblock(ls);
}

MEM__CALL int lstack__newblock(LStack *ls) {
  void *m;
  int err;
  Stack s;

  s = ls->stack;

  m = malloc(ls->block_size);
  if (!m)
    return mem_errno = MEM_FULL;

  err = stack_init(&ls->stack, m, ls->block_size);
  if (err)
    return err;

  if (!stack_push_val(&ls->stack, Stack, s))
    return mem_errno = MEM_INVALID_ARG;

  return 0;
}

MEM__CALL void* lstack_push_ex(LStack *ls, long size, int align) {
  void *p;

  p = stack_push_ex(&ls->stack, size, align);
  if (p)
    return p;
  if (mem_errno != MEM_FULL)
    return 0;

  if (!lstack__newblock(ls))
    return 0;

  return p;
}

MEM__CALL void lstack_pop(LStack *ls, void *to) {
  Stack *s;
  unsigned char *t;

  s = &ls->stack;
  t = (unsigned char*)to;
  while (t < s->begin || t >= s->end) {
    unsigned char *tmp;

    tmp = s->begin;
    *s = *(Stack*)s->begin;
    free(tmp);
  }

  s->end = to;
}

MEM__CALL void lstack_clear(LStack *ls) {
  lstack_pop(ls, ls->stack.begin);
}

MEM__CALL int lstack_num_blocks(LStack *ls) {
  Stack *s;
  int c;

  s = &ls->stack;
  for (c = 0; s->begin; ++c)
    s = (Stack*)s->begin;

  return c;
}

/* BLOCK IMPLEMENTATION */

MEM__CALL int block_init(Block *block, void *mem, long num_items, long item_size) {
  int err;

  if (item_size < (long)sizeof(void*))
    return mem_errno = MEM_INVALID_ARG;
  block->item_size = item_size;
  block->next = 0;

  err = block_add_block(block, mem, num_items);
  if (err)
    return err;

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

MEM__CALL int block_add_block(Block *block, void *mem, long num_items) {
  int i;
  unsigned char *p;

  if ((long)mem & (mem__alignof(void*)-1))
    return mem_errno = MEM_INVALID_ALIGN;

  /* set up chain pointers */
  for (p = (unsigned char*)mem, i = 0; i < num_items-1; ++i, p += block->item_size)
    *(void**)p = p+block->item_size;
  *(void**)p = block->next;

  block->next = (unsigned char*)mem;
  return 0;
}

/* LBLOCK IMPLEMENTATION */

MEM__CALL int lblock_init(LBlock *lb, long num_items, long item_size) {
  void *m;

  lb->num_items = num_items;

  m = malloc(lb->num_items * item_size);
  if (!m)
    return mem_errno = MEM_FULL;

  return block_init(&lb->block, m, num_items, item_size);
}

MEM__CALL void* lblock_get(LBlock *lb) {
  void *p;
  int err;

  p = block_get(&lb->block);
  if (p)
    return p;
  if (mem_errno != MEM_FULL)
    return 0;

  p = malloc(lb->num_items * lb->block.item_size);
  if (!p) {
    mem_errno = MEM_FULL;
    return 0;
  }

  err = block_add_block(&lb->block, p, lb->num_items);
  if (err)
    return 0;

  return block_get(&lb->block);
}

MEM__CALL void lblock_pop(LBlock *lb, void *to);


#endif /* MEM_IMPLEMENTATION */