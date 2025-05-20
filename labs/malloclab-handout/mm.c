/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memlib.h"
#include "mm.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define INIT_HEAP (ALIGNMENT * 1024)
#define INCR (ALIGNMENT * 1024) // make sure last (header + 1) is aligned

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define IS_ALIGN(header) (((size_t)(header) & 0x7) == 0)
#define IS_FREE(header) ((*(header) & 0x1) == 0)
#define IS_ALLOC(header) ((*(header) & 0x1) == 1)
#define IS_LAST(header) (*(header) == 1)
#define PREV_HEADER(header) ((header)[2])
#define NEXT_HEADER(header) ((header)[3])
#define GET_SIZE(header) (*(header) & ~0x7)

static size_t *list_root; // points to the first header!

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
    void *p = mem_sbrk(INIT_HEAP);
    assert(p != (void *)-1);
    size_t *first_header = (size_t *)p;
    size_t *last_header = (size_t *)((char *)mem_heap_hi() - ALIGNMENT + 1);
    *last_header = 1;
    assert(IS_ALLOC(last_header));
    assert(IS_ALIGN(last_header + 1));

    *first_header = INIT_HEAP - 3 * ALIGNMENT;
    assert(IS_FREE(first_header));
    // footer! for first block
    memmove((char *)last_header - ALIGNMENT, first_header, ALIGNMENT);
    PREV_HEADER(first_header) = 0;
    NEXT_HEADER(first_header) = (size_t)last_header;
    list_root = first_header;
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
    // TODO consider sbrk later
    int newsize = ALIGN(size);
    // loop to check all free list
    size_t *header = list_root;
    // find appropriate block
    while (!IS_LAST(header)) {
        if (newsize <= GET_SIZE(header)) {
            break;
        }
        // update
        header = NEXT_HEADER(header);
    }
    assert(!IS_LAST(header));
    // handle list invariant
    size_t *previous_header = PREV_HEADER(header);
    size_t *next_header = NEXT_HEADER(header);
    if (previous_header == 0) {
        list_root = next_header;
    } else {
        NEXT_HEADER(previous_header) = next_header;
    }
    if (!IS_LAST(next_header)) {
        PREV_HEADER(next_header) = previous_header;
    } // TODO this is so ugly!

    split(header, newsize);
    assert(IS_ALLOC(header));
    assert(IS_ALIGN((void *)((char *)header + SIZE_T_SIZE)));
    return (void *)((char *)header + SIZE_T_SIZE);
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
    size_t *header = (size_t *)((char *)ptr - SIZE_T_SIZE);
    assert(IS_ALLOC(header));
    // set free
    *header &= ~0x1;
    // need to add pointers!
    PREV_HEADER(header) = 0;
    NEXT_HEADER(header) = (size_t)list_root;
    // add to front of list!
    list_root = header;
    // TODO maybe coalescing! there is at most 2 to coalesce! later!
    return;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

/* helper function */
void split(size_t *header, size_t newsize) {
    size_t threshold = 8 * ALIGNMENT;
    size_t oldsize = GET_SIZE(header);
    if (newsize + threshold <= oldsize) {
        size_t *footer_left = (char *)header + newsize + SIZE_T_SIZE;
        size_t *footer_right = (char *)header + oldsize + SIZE_T_SIZE;
        size_t *header_right = (char *)footer_left + SIZE_T_SIZE;

        // set left meta-data
        *header = newsize;
        *header |= 0x1;
        memmove(footer_left, header, ALIGNMENT);
        // set right meta-data
        size_t remain_size = oldsize - newsize - 2 * SIZE_T_SIZE;
        *header_right = remain_size;
        memmove(footer_right, header_right, ALIGNMENT);
        // set pointer
        PREV_HEADER(header_right) = 0;
        NEXT_HEADER(header_right) = (size_t)list_root;
        list_root = header_right;
    }
    return;
}