/**
 * @file allocator.c
 *
 * Explores memory management at the C runtime level.
 *
 * To use (one specific command):
 * LD_PRELOAD=$(pwd)/allocator.so command
 * ('command' will run with your allocator)
 *
 * To use (all following commands):
 * export LD_PRELOAD=$(pwd)/allocator.so
 * (Everything after this point will use your custom allocator -- be careful!)
 */

#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>

#include "allocator.h"
#include "debug.h"

/**
 * print_memory
 *
 * Prints out the current memory state, including both the regions and blocks.
 * Entries are printed in order, so there is an implied link from the topmost
 * entry to the next, and so on.
 */
void print_memory(void)
{
    puts("-- Current Memory State --");
    struct mem_block *current_block = g_head;
    struct mem_block *current_region = NULL;
     while (current_block != NULL) {
        if (current_block->region_start != current_region) {
            current_region = current_block->region_start;
            printf("[REGION] %p-%p %zu\n",
                    current_region,
                    (void *) current_region + current_region->region_size,
                    current_region->region_size);
        }
        printf("[BLOCK]  %p-%p (%lu) '%s' %zu %zu %zu\n",
                current_block,
                (void *) current_block + current_block->size,
                current_block->alloc_id,
                current_block->name,
                current_block->size,
                current_block->usage,
                current_block->usage == 0
                    ? 0 : current_block->usage - sizeof(struct mem_block));
        current_block = current_block->next;
    }
}

/**
 * write_memory
 *
 * Prints out the current memory state, including both the regions and blocks.
 * Entries are printed in order, so there is an implied link from the topmost
 * entry to the next, and so on.
 * 
 * Write to memory file 
 */
void write_memory(FILE *fp)
{
    fputs("-- Current Memory State --\n", fp);
    struct mem_block *current_block = g_head;
    struct mem_block *current_region = NULL;
     while (current_block != NULL) {
        if (current_block->region_start != current_region) {
            current_region = current_block->region_start;
            fprintf(fp,"[REGION] %p-%p %zu\n",
                    current_region,
                    (void *) current_region + current_region->region_size,
                    current_region->region_size);
        }
        fprintf(fp ,"[BLOCK]  %p-%p (%lu) '%s' %zu %zu %zu\n",
                current_block,
                (void *) current_block + current_block->size,
                current_block->alloc_id,
                current_block->name,
                current_block->size,
                current_block->usage,
                current_block->usage == 0
                    ? 0 : current_block->usage - sizeof(struct mem_block));
        current_block = current_block->next;
    }
}


/**
 * reuse
 *
 *
 * @param size of malloc space to allocate
 * @param includes size of struct
 *
 * Reuses memory management alg. to resuse remaining memory 
 * If found split block
 */
void *reuse(size_t size)
{
    //size += sizeof(struct mem_block);
	if(g_head == NULL){
	    return NULL;
    }

    char *algo = getenv("ALLOCATOR_ALGORITHM");
    if(algo == NULL){
    	algo = "first_fit";
    }

	struct mem_block *current_block = g_head;
	
	if(strcmp(algo, "first_fit") == 0){
		LOGP("running first fit\n");
		 while(current_block != NULL){

			if((current_block->size - current_block->usage) >= size + sizeof(struct mem_block)){
              if(current_block -> usage != 0){
                LOGP("splitting block\n");

                //struct mem_block *fit_block = helper_addfit(current_block, size);
                return helper_addfit(current_block, size);

                }
            else{
                LOGP("not splitting, updating block\n");
                current_block->alloc_id = g_allocations++;
                strcpy(current_block->name, "hoi");
                current_block->usage = size + sizeof(struct mem_block);
                return current_block;
                }
			}
			 current_block = current_block->next;
		 }

	}else if(strcmp(algo, "best_fit") == 0){
        bool flag_best = false;
        struct mem_block *min_block = NULL;
        int min_block_sz = current_block->size - current_block->usage;
        while(current_block != NULL){
            
            if((current_block->size - current_block->usage) >= size + sizeof(struct mem_block)){
                if(((current_block->size - current_block->usage) < min_block_sz) || flag_best == false){
                        min_block_sz = current_block->size - current_block->usage;
                        min_block = current_block;
                        flag_best = true;
                }
            }
            current_block = current_block->next;
        }

        if(flag_best){
            if(min_block -> usage != 0){
                return helper_addfit(min_block, size);
            }
            else{
                min_block->alloc_id = g_allocations++;
                min_block->usage = size;
                strcpy(min_block->name, "hoi");
                return min_block;
            }
        }

        }else if(strcmp(algo, "worst_fit") == 0){
        bool flag_best = false;
        struct mem_block *max_block = NULL;
        int max_block_sz = current_block->size - current_block->usage;
        while(current_block != NULL){
            
            if((current_block->size - current_block->usage) >= size + sizeof(struct mem_block)){
                if(((current_block->size - current_block->usage) > max_block_sz) || flag_best == false){
                        max_block_sz = current_block->size - current_block->usage;
                        max_block = current_block;
                        flag_best = true;
                }
            }
            current_block = current_block->next;
        }

        if(flag_best){
            if(max_block -> usage != 0){
                return helper_addfit(max_block, size);
            }
            else{
                max_block->alloc_id = g_allocations++;
                max_block->usage = size;
                strcpy(max_block->name, "hoi");
                return max_block;
            }
        }
    }
	return NULL;	
}

/**
 * helper_addfit
 *
 *
 * @param f_block - current block that will split
 * @param Size - size to malloc
 *
 * Split Block
 */
struct mem_block *helper_addfit(struct mem_block *f_block, size_t size){


    struct mem_block *new_block = (void*)f_block + f_block->usage;

    LOGP("new block created\n");
    new_block->size = f_block->size - f_block->usage;

    LOGP("new block created 2\n");

    new_block->usage = size + sizeof(struct mem_block);


    new_block->alloc_id = g_allocations++;
    new_block->next = f_block->next;

    strcpy(new_block->name, "hoi");

    LOG("Region Start %p\n",f_block->region_start);
    new_block->region_start = f_block->region_start;
   
    f_block->size = f_block->usage;



    f_block->next = new_block;

    LOGP("bloop\n");
    return new_block;
}




/**
 * Malloc
 *
 *
 * @param Size - size to malloc
 *
 * Creates a new Region Block and assigned values to mem_block Struct
 * Before malloc check reuse to re-use memory
 */
void *malloc(size_t size)
{
    LOG("Allocation request with size = %zu\n", size);

    if(size % 8 != 0) size += 8 - size % 8;
    
    
    int scrib = 0;
    if(getenv("ALLOCATOR_SCRIBBLE") != NULL){
         scrib = atoi(getenv("ALLOCATOR_SCRIBBLE"));
     }

    struct mem_block *ptr = reuse(size);
    if(ptr != NULL){

        if(scrib){
            memset(ptr + 1,0xAA, size);
        }
    }
    if(ptr != NULL){
        return ptr +1;
    }

    size_t actual_sz = size + sizeof(struct mem_block);

    int page_sz = getpagesize();
    size_t num_pages = actual_sz / page_sz;


    if(actual_sz % page_sz != 0){
        num_pages++;
    }

    struct mem_block *block = mmap(
        NULL, /* Let the kernal decide where this goes */
        num_pages * page_sz, /* Do we need a comment for this? */
        PROT_READ | PROT_WRITE, /* Protection flags */
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1, /* This is not a file, it's just anoy. entry */
        0);

    if(block == MAP_FAILED){
        perror("mmap");
        return NULL;
    }


    block->alloc_id = g_allocations++;
    strcpy(block->name, "hoi");
    block->size = num_pages * page_sz;
    block->usage = actual_sz;/*changed from size */
    block->region_start = block;
    block->region_size = num_pages * page_sz;
    block->next = NULL;

    if(g_head == NULL){
    	g_head = block;
    }
    else{
        struct mem_block *current_block = g_head;
        while(current_block->next != NULL){
            current_block = current_block->next;
        }
        current_block->next = block;
    }

    if(scrib){
       memset(block+1 ,0xAA, size);
     }

    return block + 1;
}


/**
 * malloc_name
 *
 * @param Size - size to malloc
 * @param Name - set name to new_block region
 *
 * Creates a new Region Block and assigned values to mem_block Struct
 */
void *malloc_name(size_t size, char *name)

{
    LOG("Allocation request with size = %zu\n", size);


    struct mem_block *ptr = malloc(size);
    struct mem_block *block = ((struct mem_block *)ptr -1);
    if(block != NULL){
        strcpy(block->name, name);
    }

    return ptr;
}



/**
 * Free
 *
 *
 * @param ptr - pointer to memory block to be free. 
 * 
 * Free Memory block -> usage = 0
 * If all the mem blocks in region is free, munmap syscall
 */
void free(void *ptr){
    if (ptr == NULL) {
         //Freeing a NULL pointer does nothing 
        return;
    }

    struct mem_block *block = ((struct mem_block *) ptr -1);

    block -> usage = 0;

    bool is_not_free = true;


  struct mem_block *free = block->region_start;

  while(free->next != NULL){
    LOGP("reaching here\n");
    if(free->next->region_start == block->region_start){
        if(free->usage != 0){
            is_not_free = false;
            break;
        }
        free = free->next;
    }else{
        break;
    }
  }

if(free->usage != 0){
    is_not_free = false;
}

if(is_not_free){

    LOGP("bloop\n");
    if(block == g_head){
        if(free->next == NULL){
          g_head = NULL;
        } else {
          g_head = free->next;
        }
        int ret = munmap(block->region_start, block->region_size);
        if(ret == -1){
          perror("munmap");
        }
      } else {
        struct mem_block *current_block = g_head;
        while(current_block->next->region_start != free->region_start){
            current_block = current_block->next;
        }
        current_block->next = free->next;
        int ret = munmap(block->region_start, block->region_size);
        if(ret == -1){
          perror("munmap");
        }
      }
    }
}





/**
 * calloc
 *
 *
 * @param nmemb
 * @param size - size passed in from user to calloc 
 *
 */
void *calloc(size_t nmemb, size_t size)
{
    void *ptr = malloc(nmemb * size);
    memset(ptr, 0, nmemb * size);
    // TODO: hmm, what does calloc do?
    return ptr;
}



/**
 * realloc
 *
 *
 * @param ptr - pointer of struct
 * @param size - size to 
 *
 * Resizes block in position that was prev. allocated. 
 */
void *realloc(void *ptr, size_t size)
{
    if (ptr == NULL) {
        /* If the pointer is NULL, then we simply malloc a new block */
        return malloc(size);
    }

    if (size == 0) {
        /* Realloc to 0 is often the same as freeing the memory block... But the
         * C standard doesn't require this. We will free the block and return
         * NULL here. */
        free(ptr);
        return NULL;
    }

    if(size % 8 != 0) size += 8 - size % 8;

    // TODO: reallocation logic

    struct mem_block *block = ((struct mem_block *) ptr - 1);

    if(block->size >= size+sizeof(struct mem_block)){
        block->usage = size+sizeof(struct mem_block);
        return ptr;
    }
    else{
        void* new_block = malloc(size);
        memcpy(new_block, ptr, block->usage-sizeof(struct mem_block));
        free(ptr);
        return new_block;
    }

    return NULL;
}


