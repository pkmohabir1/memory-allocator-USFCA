/**
 * @file allocator.h
 *
 * Function prototypes and globals for our memory allocator implementation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#ifndef ALLOCATOR_H
#define ALLOCATOR_H

/* -- Helper functions -- */
void print_memory(void);
void *reuse(size_t size);

/* -- C Memory API functions -- */
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
struct mem_block *helper_addfit(struct mem_block *fit_block, size_t size);
void write_memory(FILE *fp);

/* -- Data Structures and Globals -- */

/**
 * Defines metadata structure for both memory 'regions' and 'blocks.' This
 * structure is prefixed before each allocation's data area.
 */
struct mem_block {
    /**
     * Each allocation is given a unique ID number. If an allocation is split in
     * two, then the resulting new block will be given a new ID.
     */
    unsigned long alloc_id;

    /**
     * The name of this memory block. If the user doesn't specify a name for the
     * block, it should be auto-generated based on the allocation ID.
     */
    char name[32];

    /** Size of the memory region */
    size_t size;

    /** Space used; if usage == 0, then the block has been freed. */
    size_t usage;

    /**
     * Pointer to the start of the mapped memory region. This simplifies the
     * process of finding where memory mappings begin.
     */
    struct mem_block *region_start;

    /**
     * If this block is the beginning of a mapped memory region, the region_size
     * member indicates the size of the mapping. In subsequent (split) blocks,
     * this is undefined.
     */
    size_t region_size;

    /** Next block in the chain */
    struct mem_block *next;

};

static struct mem_block *g_head = NULL; /*!< Start (head) of our linked list */
static unsigned long g_allocations = 0; /*!< Allocation counter */

#endif
