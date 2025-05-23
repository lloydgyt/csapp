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
#define INIT_HEAP (ALIGNMENT * (1 << 10))
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

static size_t *list_root_0; // points to the bigger list
static size_t *list_root_1; // points to the small list
static char *heap_low;      // points to the first byte in heap
static char *heap_high;     // points to the last byte in heap
static size_t malloc_counter = 0;
static size_t free_counter = 0;
// static size_t realloc_counter = 0;

/* prototype */
void split(size_t *header, size_t newsize);
void extract_node(size_t *header);
void head_insert(size_t *header);
size_t *expand(size_t request_size);
void mark_free(size_t *header, size_t *footer);
void mark_allocated(size_t *header, size_t *footer);
void set_size(size_t *header, size_t *footer, size_t newsize);
size_t *get_root(size_t size);
size_t **get_root_pointer(size_t size);
void coalesce_right(size_t *header, size_t **footer_p);
void coalesce_left(size_t **header_p, size_t *footer);
void DS_consistency_checker();
void list_consistency_checker(size_t *root);

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
    void *p = mem_sbrk(INIT_HEAP);
    assert(p != (void *)-1);
    heap_low = (char *)p;
    heap_high = (char *)mem_heap_hi();
    size_t *first_header = (size_t *)p;
    size_t *first_footer = (size_t *)(heap_high - (ALIGNMENT) + 1);
    set_size(first_header, first_footer, INIT_HEAP - 2 * ALIGNMENT);

    // trick: put last header right 'in' the first header!
    size_t *last_header = (size_t *)((char *)first_header + sizeof(size_t));
    *last_header = 1;

    assert(IS_FREE(first_header));
    PREV_HEADER(first_header) = 0;
    NEXT_HEADER(first_header) = (size_t)last_header;
    // TODO seglist
    list_root_0 = first_header;
    list_root_1 = last_header;
    DS_consistency_checker();
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
    // TODO how to handle this?
    assert(size != 0);

    malloc_counter++;
    // printf("malloc times = %u\n", malloc_counter++);
    int newsize = ALIGN(size);
    // loop to check all free list
    // TODO seglist
    size_t *header = get_root(newsize);
    while (!IS_LAST(header)) {
        assert(IS_FREE(header));
        if (newsize <= GET_SIZE(header)) {
            break;
        }
        // update
        header = (size_t *)NEXT_HEADER(header);
    }
    // can't find block, expand heap
    // TODO if not found, should go to next pointer first!
    if (IS_LAST(header)) {
        header = expand(newsize);
    }
    if (malloc_counter == 10) {
        DS_consistency_checker();
    }

    assert(IS_FREE(header) && (newsize <= GET_SIZE(header)));
    split(header, newsize);
    extract_node(header);
    mark_allocated(header, GET_FOOTER_FROM_HEADER(header));
    assert(IS_ALLOC(header));
    assert(IS_ALIGN((void *)((char *)header + SIZE_T_SIZE)));
    assert(GET_SIZE(header) >= newsize);

    DS_consistency_checker();
    return (void *)((char *)header + SIZE_T_SIZE);
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
    free_counter++;
    // printf("free times = %u\n", free_counter++);
    size_t *header = (size_t *)((char *)ptr - SIZE_T_SIZE);
    assert(IS_ALLOC(header));
    size_t size = GET_SIZE(header);
    size_t *footer = (size_t *)((char *)header + size + SIZE_T_SIZE);
    mark_free(header, footer);

    coalesce_right(header, &footer);
    DS_consistency_checker();
    coalesce_left(&header, footer);
    DS_consistency_checker();
    assert(IS_FREE(header));
    head_insert(header);

    DS_consistency_checker();
    return;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
    assert(ptr && size);
    // TODO actually there is more requirement!
    // what if ptr is NULL??
    // what if size is 0??

    // printf("realloc times = %u\n", realloc_counter++);
    size_t *old_header = (size_t *)((char *)ptr - ALIGNMENT);
    assert(IS_ALLOC(old_header));
    size_t old_size = GET_SIZE(old_header);
    size_t new_size = ALIGN(size);
    // find a bigger block
    // how to assert malloc? malloc guarantees it!
    void *new_ptr = mm_malloc(new_size);

    // copy content min(old, new) bytes to new block
    size_t copy_size = MIN(old_size, new_size);
    memmove(new_ptr, ptr, copy_size);

    // free the old block
    mm_free(ptr);
    DS_consistency_checker();
    return new_ptr;
}

/* HELPER FUNCTION */
/*
    may split a free into 2, insert the right free block
*/
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
        set_size(header, footer_left, newsize);

        // set right meta-data
        size_t remain_size = oldsize - newsize - 2 * SIZE_T_SIZE;
        set_size(header_right, footer_right, remain_size);
        // set pointer (header insert)
        head_insert(header_right);
        assert(IS_FREE(header_right));
        if (malloc_counter == 10) {
            DS_consistency_checker();
        }
    }
    return;
}

// handle list invariant, take node out of list
void extract_node(size_t *header) {
    size_t *previous_header = (size_t *)PREV_HEADER(header);
    size_t *next_header = (size_t *)NEXT_HEADER(header);
    if (previous_header == 0) {
        // TODO seglist
        size_t **rootp = get_root_pointer(GET_SIZE(header));
        *rootp = next_header;
    } else {
        NEXT_HEADER(previous_header) = (size_t)next_header;
    }

    if (!IS_LAST(next_header)) {
        PREV_HEADER(next_header) = (size_t)previous_header;
    } // TODO this is so ugly! use dummy node to refactor!
}

// TODO should also pass size!
void head_insert(size_t *header) {
    // TODO seglist
    size_t size = GET_SIZE(header);
    size_t *next = get_root(size);

    PREV_HEADER(header) = 0;
    NEXT_HEADER(header) = (size_t)next;
    // TODO seglist
    size_t **rootp = get_root_pointer(size);
    *rootp = header;
    if (!IS_LAST(next)) {
        PREV_HEADER(next) = (size_t)header;
    }
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
    set_size(header, footer, newsize);

    coalesce_left(&header, footer);
    assert(IS_FREE(header));
    head_insert(header);
    return header;
}

void mark_free(size_t *header, size_t *footer) {
    *header &= ~0x1;
    memmove(footer, header, ALIGNMENT);
}

void mark_allocated(size_t *header, size_t *footer) {
    *header |= 0x1;
    memmove(footer, header, ALIGNMENT);
}

void set_size(size_t *header, size_t *footer, size_t newsize) {
    *header = newsize;
    memmove(footer, header, ALIGNMENT);
}

size_t *get_root(size_t size) {
    // TODO change later
    if (size > 128 * ALIGNMENT) {
        return list_root_0;
    } else {
        return list_root_1;
    }
}
size_t **get_root_pointer(size_t size) {
    // TODO change later
    if (size > 128 * ALIGNMENT) {
        return &list_root_0;
    } else {
        return &list_root_1;
    }
}

/*
    merge current free block with right block
    get a big free block, pointer is not set yet
*/
void coalesce_right(size_t *header, size_t **footer_p) {
    assert(IS_FREE(header));
    size_t size = GET_SIZE(header);
    size_t *footer = *footer_p;
    size_t *right_header = (size_t *)((char *)footer + SIZE_T_SIZE);
    if (!IS_HIGH(footer) && IS_FREE(right_header)) {
        size_t *right_footer = GET_FOOTER_FROM_HEADER(right_header);
        assert(IS_FREE(right_footer));
        assert(GET_SIZE(right_header) == GET_SIZE(right_footer));

        // extract node of right header
        extract_node(right_header);
        footer = right_footer;
        set_size(header, footer,
                 size + GET_SIZE(right_header) + 2 * SIZE_T_SIZE);
        *footer_p = footer;
    } else {
        assert(IS_HIGH(footer) || IS_ALLOC(right_header)); // TODO this is good
    }
}

/*
    merge current free block with left block
    get a big free block, pointer is not set yet
*/
void coalesce_left(size_t **header_p, size_t *footer) {
    size_t *header = *header_p;
    assert(IS_FREE(header));
    size_t size = GET_SIZE(header);
    size_t *left_footer = (size_t *)((char *)header - SIZE_T_SIZE);
    if (!IS_LOW(header) && IS_FREE(left_footer)) {
        size_t *left_header = GET_HEADER_FROM_FOOTER(left_footer);
        assert(IS_FREE(left_header));
        assert(GET_SIZE(left_footer) == GET_SIZE(left_header));

        extract_node(left_header);
        header = left_header;
        set_size(header, footer,
                 size + GET_SIZE(left_header) + 2 * SIZE_T_SIZE);
        *header_p = header;
    } else {
        assert(IS_LOW(header) || IS_ALLOC(left_footer));
    }
}

void DS_consistency_checker() {
    list_consistency_checker(list_root_0);
    list_consistency_checker(list_root_1);
}

void list_consistency_checker(size_t *root) {
    size_t *header = root;
    while (!IS_LAST(header)) {
        size_t *footer = GET_FOOTER_FROM_HEADER(header);
        assert(IS_FREE(header));
        assert(GET_SIZE(header) == GET_SIZE(footer));
        // update
        header = (size_t *)NEXT_HEADER(header);
    }
}
