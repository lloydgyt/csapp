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
#define INIT_HEAP (ALIGNMENT * (1 << 10)) // TODO should be bigger!
#define INCR (1 << 10)
#define ROUND_UP(size) (((size) + (INCR) - 1) & ~((INCR) - 1))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

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
#define GET_FOOTER_FROM_HEADER(header)                                         \
    ((size_t *)((char *)(header) + (GET_SIZE(header)) + (SIZE_T_SIZE)))
#define GET_HEADER_FROM_FOOTER(footer)                                         \
    ((size_t *)((char *)(footer) - (GET_SIZE(footer)) - (SIZE_T_SIZE)))
#define IS_LOW(header) ((char *)(header) == heap_low)
#define IS_HIGH(header) ((char *)(header) == (heap_high - (ALIGNMENT) + 1))

static size_t *list_root; // points to the first header!
static char *heap_low;    // points to the first byte in heap
static char *heap_high;   // points to the last byte in heap

/* prototype */
void split(size_t *header, size_t newsize);
void extract_node(size_t *header);
void head_insert(size_t *header);
size_t *expand(size_t request_size);

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
    void *p = mem_sbrk(INIT_HEAP);
    assert(p != (void *)-1);
    heap_low = (char *)p;
    heap_high = (char *)mem_heap_hi();
    size_t *first_header = (size_t *)p;
    // trick: put last header right 'in' the first header!
    size_t *last_header = (size_t *)((char *)first_header + sizeof(size_t));
    *last_header = 1;

    *first_header = INIT_HEAP - 2 * ALIGNMENT;
    assert(IS_FREE(first_header));
    // footer! for first block
    memmove(heap_high - (ALIGNMENT) + 1, first_header, ALIGNMENT);
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
    // TODO how to handle this?
    assert(size != 0);
    // static size_t counter = 0;
    // printf("\n");
    // printf("malloc times = %u\n", counter++);
    int newsize = ALIGN(size);
    // loop to check all free list
    size_t *header = list_root;
    while (!IS_LAST(header)) {
        // printf("times = %u, request_size = %u, current_size = %u\n", counter,
        //        newsize, GET_SIZE(header));
        if (newsize <= GET_SIZE(header)) {
            break;
        }
        // update
        header = (size_t *)NEXT_HEADER(header);
    }
    // can't find block, expand heap
    if (IS_LAST(header)) {
        header = expand(newsize);
    }

    assert(IS_FREE(header) && (newsize <= GET_SIZE(header)));
    extract_node(header);
    *header |= 0x1; // mark as used
    split(header, newsize);
    assert(IS_ALLOC(header));
    assert(IS_ALIGN((void *)((char *)header + SIZE_T_SIZE)));
    assert(GET_SIZE(header) >= newsize);
    return (void *)((char *)header + SIZE_T_SIZE);
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
    // static size_t counter = 0;
    // printf("\n");
    // printf("free times = %u\n", counter++);
    size_t *header = (size_t *)((char *)ptr - SIZE_T_SIZE);
    assert(IS_ALLOC(header));
    size_t size = GET_SIZE(header);
    size_t *footer = (size_t *)((char *)header + size + SIZE_T_SIZE);

    // merge right only change footer
    size_t *right_header = (size_t *)((char *)footer + SIZE_T_SIZE);
    if (!IS_HIGH(footer) && IS_FREE(right_header)) {
        // extract node of right header
        extract_node(right_header);
        // set header
        size = size + GET_SIZE(right_header) + 2 * SIZE_T_SIZE;
        *header = size;
        // set footer
        size_t *right_footer = GET_FOOTER_FROM_HEADER(right_header);
        assert(IS_FREE(right_footer));
        footer = right_footer;
        memmove(footer, header, ALIGNMENT);
        // not yet head insert
    } else {
        assert(IS_HIGH(footer) || IS_ALLOC(right_header)); // TODO this is good
    }

    // merge left only changes header
    size_t *left_footer = (size_t *)((char *)header - SIZE_T_SIZE);
    if (!IS_LOW(header) && IS_FREE(left_footer)) {
        // set header
        size_t *left_header = GET_HEADER_FROM_FOOTER(left_footer);
        assert(IS_FREE(left_header));
        header = left_header;
        *header = GET_SIZE(left_header) + size + 2 * SIZE_T_SIZE;
        memmove(footer, header, ALIGNMENT);
        // the node stays the same!
    } else {
        assert(IS_LOW(header) || IS_ALLOC(left_footer));
        head_insert(header);
    }
    // set free
    *header &= ~0x1;
    return;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
    // TODO actually there is more requirement!
    // what if ptr is NULL??
    // what if size is 0??
    assert(ptr && size);
    size_t *header = (size_t *)((char *)ptr - ALIGNMENT + 1);
    assert(IS_ALLOC(header));
    size_t old_size = GET_SIZE(header);
    size_t new_size = ALIGN(size);
    // find a bigger block
    // how to assert malloc? malloc guarantees it!
    void *new_ptr = mm_malloc(new_size);

    // copy content min(old, new) bytes to new block
    size_t copy_size = MIN(old_size, new_size);
    memmove(new_ptr, ptr, copy_size);

    // free the old block
    mm_free(ptr);
    return new_ptr;
}

/* helper function */
void split(size_t *header, size_t newsize) {
    size_t threshold = 8 * ALIGNMENT;
    size_t oldsize = GET_SIZE(header);
    if (newsize + threshold <= oldsize) {
        size_t *footer_left =
            (size_t *)((char *)header + newsize + SIZE_T_SIZE);
        size_t *footer_right =
            (size_t *)((char *)header + oldsize + SIZE_T_SIZE);
        size_t *header_right = (size_t *)((char *)footer_left + SIZE_T_SIZE);

        // set left meta-data
        *header = newsize;
        *header |= 0x1;
        memmove(footer_left, header, ALIGNMENT);
        // set right meta-data
        size_t remain_size = oldsize - newsize - 2 * SIZE_T_SIZE;
        *header_right = remain_size;
        memmove(footer_right, header_right, ALIGNMENT);
        // set pointer (header insert)
        head_insert(header_right);
    }
    return;
}

// handle list invariant, take node out of list
void extract_node(size_t *header) {
    size_t *previous_header = (size_t *)PREV_HEADER(header);
    size_t *next_header = (size_t *)NEXT_HEADER(header);
    if (previous_header == 0) {
        list_root = next_header;
    } else {
        NEXT_HEADER(previous_header) = (size_t)next_header;
    }
    if (!IS_LAST(next_header)) {
        PREV_HEADER(next_header) = (size_t)previous_header;
    } // TODO this is so ugly! use dummy node to refactor!
}

void head_insert(size_t *header) {
    PREV_HEADER(header) = 0;
    NEXT_HEADER(header) = (size_t)list_root;
    list_root = header;
}

/* expand the heap and return a pointer that is ready to use */
size_t *expand(size_t request_size) {
    size_t expand_size = ROUND_UP(request_size);
    void *p = mem_sbrk(expand_size);
    assert(p != (void *)-1);
    heap_high = (char *)mem_heap_hi();

    // set block metadata (except pointers)
    size_t *header = (size_t *)p;
    size_t *footer = (size_t *)(heap_high - ALIGNMENT + 1);
    size_t newsize = expand_size - 2 * ALIGNMENT;
    *header = newsize;
    assert(IS_FREE(header));
    memmove(footer, header, ALIGNMENT);

    // maybe merge left
    // TODO may wrap this (footer needs to be set before!)
    size_t *left_footer = (size_t *)((char *)header - SIZE_T_SIZE);
    if (!IS_LOW(header) && IS_FREE(left_footer)) {
        // set header
        size_t *left_header = GET_HEADER_FROM_FOOTER(left_footer);
        assert(IS_FREE(left_header));
        header = left_header;
        *header = GET_SIZE(left_header) + newsize + 2 * SIZE_T_SIZE;
        memmove(footer, header, ALIGNMENT);
        // the node stays the same!
    } else {
        assert(IS_LOW(header) || IS_ALLOC(left_footer));
        head_insert(header);
    }
    // set free
    *header &= ~0x1;
    return header;
}