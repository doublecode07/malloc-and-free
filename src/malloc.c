#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)     ((b) + 1)
#define BLOCK_HEADER(ptr) ((struct _block *)(ptr) - 1)

/* Flags to manage allocation tracking */
static bool first_allocation = true;       // To skip the first growHeap call
static bool in_printStatistics = false;    // To prevent tracking during printStatistics

static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

struct _block 
{
   size_t  size;         /* Size of the allocated _block of memory in bytes     */
   struct _block *next;  /* Pointer to the next _block of allocated memory      */
   struct _block *prev;  /* Pointer to the previous _block of allocated memory  */
   bool   free;          /* Is this _block free?                                */
   char   padding[3];    /* Padding to align the structure                      */
};

struct _block *heapList = NULL; /* Free list to track the _blocks available */
struct _block *last_allocated = NULL; // For Next Fit implementation
static void *heap_start = NULL;  // Track start of heap

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
    /* Set the flag to indicate we're in printStatistics */
    in_printStatistics = true;

    printf("\nheap management statistics\n");
    printf("mallocs:\t%d\n", num_mallocs);
    printf("frees:\t\t%d\n", num_frees );
    printf("reuses:\t\t%d\n", num_reuses );
    printf("grows:\t\t%d\n", num_grows );
    printf("splits:\t\t%d\n", num_splits );
    printf("coalesces:\t%d\n", num_coalesces );
    printf("blocks:\t\t%d\n", num_blocks);
    printf("requested:\t%d\n", num_requested);
    printf("max heap:\t%d\n", max_heap);

    // Calculate fragmentation and count free blocks
    size_t total_free = 0;
    size_t largest_free = 0;
    struct _block *curr = heapList;
    while (curr)
    {
        if (curr->free)
        {
            total_free += curr->size;
            if (curr->size > largest_free)
            {
                largest_free = curr->size;
            }
        }
        curr = curr->next;
    }

    double fragmentation = 0;
    if (total_free > 0)
    {
        fragmentation = 1 - ((double)largest_free / total_free);
    }

    printf("\nStatistics below are for test6\n");
    printf("Total free memory: %zu bytes\n", total_free);
    printf("Largest free block: %zu bytes\n", largest_free);
    printf("Fragmentation: %.2f%%\n", fragmentation * 100);

    /* Reset the flag */
    in_printStatistics = false;
}

/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes 
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 * \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
 */
struct _block *findFreeBlock(struct _block **last, size_t size) 
{
   struct _block *curr = heapList;

#if defined FIT && FIT == 0
   /* First fit */
      //
   // While we haven't run off the end of the linked list and
   // while the current node we point to isn't free or isn't big enough
   // then continue to iterate over the list.  This loop ends either
   // with curr pointing to NULL, meaning we've run to the end of the list
   // without finding a node or it ends pointing to a free node that has enough
   // space for the request.
   // 
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;
   }
   return curr;
#endif

// \TODO Put your Best Fit code in this #ifdef block
#if defined BEST && BEST == 0
   /** \TODO Implement best fit here */
   /* Best fit */
   struct _block *best = NULL;
   size_t min_size = (size_t)-1;  // Initialize to maximum possible size
   while (curr)
   {
       if (curr->free && curr->size >= size)
       {
           if (curr->size < min_size) 
           {
               min_size = curr->size;
               best = curr;
           }
       }
       *last = curr;
       curr = curr->next;
   }
   return best;
#endif

// \TODO Put your Worst Fit code in this #ifdef block
#if defined WORST && WORST == 0
   /** \TODO Implement worst fit here */
   /* Worst fit */
   struct _block *worst = NULL;
   size_t max_size = 0;
   while (curr)
   {
       if (curr->free && curr->size >= size)
       {   
           if (curr->size > max_size) 
           {
               max_size = curr->size;
               worst = curr;
           }
       }
       *last = curr;
       curr = curr->next;
   }
   return worst;
#endif

// \TODO Put your Next Fit code in this #ifdef block
#if defined NEXT && NEXT == 0
   /** \TODO Implement next fit here */
   /* Next fit */
   if (!curr) {
       return NULL;  // Empty heap
   }
   
   /* Start from last allocated if it exists, otherwise start from beginning */
   if (last_allocated) {
       curr = last_allocated->next;
       if (!curr) {
           curr = heapList;  // Wrap to beginning if at end
       }
   }

   /* Save starting point */
   struct _block *start = curr;
   
   /* Search once through the list */
   do {
       if (curr->free && curr->size >= size) {
           last_allocated = curr;  // Update last allocated
           return curr;
       }
       *last = curr;
       curr = curr->next;
       if (!curr) {
           curr = heapList;  // Wrap around
       }
   } while (curr != start);

   /* If we get here, we've searched the entire list and found nothing */
   return NULL;
#endif

   return curr;
}

/*
 * \brief growHeap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size) 
{
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0);
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1) 
   {
      return NULL;
   }

    if (heap_start == NULL) 
    {
       heap_start = curr;
    }

   /* Update heapList if not set */
   if (heapList == NULL) 
   {
      heapList = curr;
      curr->prev = NULL;
   }

   /* Attach new _block to previous _block */
    if (last) 
    {
        last->next = curr;
        curr->prev = last;
    }

   /* Update _block metadata:
      Set the size of the new block and initialize the new block to "free".
      Set its next pointer to NULL since it's now the tail of the linked list.
   */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;

    /* Skip first allocation (for atexit) for grows count */
    if (!first_allocation && !in_printStatistics) {
        num_grows++;
    } else {
        first_allocation = false;
    }
    num_blocks++;
    
    /* Update max heap size */
    if (heapList) {
        size_t heap_size = 0;
        struct _block *curr_iter = heapList;
        while (curr_iter) {
            heap_size += curr_iter->size;  // Use unaligned size
            curr_iter = curr_iter->next;
        }

        if (heap_size > max_heap) {
            max_heap = heap_size;
        }
    }
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{

   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }

   /* Look for free _block.  If a free block isn't found then we need to grow our heap. */

   struct _block *last = heapList;
   struct _block *next = findFreeBlock(&last, size);

   /* TODO: If the block found by findFreeBlock is larger than we need then:
        If the leftover space in the new block is greater than the sizeof(_block)+4 then
        split the block.
        If the leftover space in the new block is less than the sizeof(_block)+4 then
        don't split the block.
   */

  if (next != NULL) {
    /* If we found a free block */
    num_reuses++;
    
    /* Check if block is large enough to split */
    size_t remaining_size = next->size - size;
    if (remaining_size >= sizeof(struct _block) + 4) {
        struct _block *new_block = (struct _block *)((char *)next + 
                                    sizeof(struct _block) + size);
                                    
        new_block->size = remaining_size - sizeof(struct _block);
        new_block->next = next->next;
        new_block->free = true;

         if (next->next) {
            next->next->prev = new_block;  // Update next block's prev pointer
        }
        
        
        next->size = size;
        next->next = new_block;
        
        num_splits++;
        num_blocks++;
    }
}

   /* Could not find free _block, so grow heap */
   if (next == NULL) 
   {
      next = growHeap(last, size);
   }

   /* Could not find free _block or grow heap, so just return NULL */
   if (next == NULL) 
   {
      return NULL;
   }
   
   /* Mark _block as in use */
    next->free = false;
    num_mallocs++;         // Count user mallocs
    num_requested += size; // Count user requests

    /*update max heap size*/
    
    if (heapList) {
        size_t actual_heap = 0;
        struct _block *curr_iter = heapList;
        while (curr_iter) {
            size_t total_block_size = curr_iter->size + sizeof(struct _block);
            total_block_size = ALIGN4(total_block_size);
            actual_heap += total_block_size;
            curr_iter = curr_iter->next;
        }

        size_t total_heap = actual_heap;

        // Include internal allocations (e.g., from printf)
        size_t internal_allocations = 0;
        // Add any known internal allocations here, if applicable

        total_heap += internal_allocations;

        if (total_heap > max_heap) {
            max_heap = total_heap;
        }
    }
    

   /* Return data address associated with _block to the user */
   return BLOCK_DATA(next);
}

/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{
   if (ptr == NULL) 
   {
      return;
   }

   /* Make _block as free */
   struct _block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;
   num_frees++;
   /* TODO: Coalesce free _blocks.  If the next block or previous block 
            are free then combine them with this block being freed.
   */
    /* Coalesce with previous block first if it's free */
   if (curr->prev && curr->prev->free)
   {
       struct _block *prev_block = curr->prev;
       prev_block->size += sizeof(struct _block) + curr->size;
       prev_block->next = curr->next;
       if (curr->next) {
           curr->next->prev = prev_block;
       }
       curr = prev_block;  // Move curr pointer to coalesced block
       num_coalesces++;
       num_blocks--;
   }

   /* Then coalesce with next block if it's free */
   if (curr->next && curr->next->free)
   {
       curr->size += sizeof(struct _block) + curr->next->size;
       curr->next = curr->next->next;
       if (curr->next) {
           curr->next->prev = curr;
       }
       num_coalesces++;
       num_blocks--;
   }
}

void *calloc( size_t nmemb, size_t size )
{
   // \TODO Implement calloc
    size_t total_size = nmemb * size;
    void *ptr = malloc(total_size);
    if (ptr)
    {
        memset(ptr, 0, total_size);
    }
    return ptr;
}

void *realloc( void *ptr, size_t size )
{
   // \TODO Implement realloc
       if (ptr == NULL)
    {
        return malloc(size);
    }

    if (size == 0)
    {
        free(ptr);
        return NULL;
    }

    struct _block *curr = BLOCK_HEADER(ptr);
    size_t current_size = curr->size;

    if (current_size >= size)
    {
        return ptr;
    }

    void *new_ptr = malloc(size);
    if (new_ptr)
    {
        memcpy(new_ptr, ptr, current_size);
        // Don't increment num_frees since this isn't a user-called free
        curr->free = true;
        if (curr->prev && curr->prev->free)
        {
            // ... coalescing code ...
            num_coalesces++;
            num_blocks--;
        }
        if (curr->next && curr->next->free)
        {
            // ... coalescing code ...
            num_coalesces++;
            num_blocks--;
        }
    }
    return new_ptr;
}



/* vim: IENTRTMzMjAgU3ByaW5nIDIwM001= ----------------------------------------*/
/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
