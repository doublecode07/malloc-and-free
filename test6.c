#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_ALLOCS 20
#define NUM_SIZES 7

int main()
{
    printf("Test6: Performance and Heap Fragmentation\n");

    // Measure start time
    clock_t start_time = clock();

    // Sizes for allocations (varying sizes)
    size_t alloc_sizes[NUM_SIZES] = {128, 256, 512, 1024, 2048, 4096, 8192};

    // Array to hold allocated pointers
    void *allocs[NUM_ALLOCS];

    // Step 1: Allocate a series of blocks of varying sizes
    for (int i = 0; i < NUM_ALLOCS; i++)
    {
        // Cycle through sizes
        size_t size = alloc_sizes[i % NUM_SIZES];
        allocs[i] = malloc(size);
        if (allocs[i] == NULL)
        {
            fprintf(stderr, "malloc failed at iteration %d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    // Step 2: Free specific blocks to create free blocks of varying sizes
    // Free every third block to create fragmentation
    for (int i = 2; i < NUM_ALLOCS; i += 3)
    {
        free(allocs[i]);
        allocs[i] = NULL; // Set pointer to NULL to avoid double-free
    }

    // Step 3: Allocate new blocks that could fit into multiple free blocks
    // These allocations are designed to test how different strategies choose blocks
    void *test_allocs[5];
    size_t test_sizes[5] = {500, 2000, 1000, 3000, 6000}; // Sizes that could fit into multiple free blocks

    for (int i = 0; i < 5; i++)
    {
        test_allocs[i] = malloc(test_sizes[i]);
        if (test_allocs[i] == NULL)
        {
            fprintf(stderr, "Test allocation failed at iteration %d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    // Measure end time
    clock_t end_time = clock();
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Execution time: %f seconds\n", time_spent);

    // Attempt to allocate a very large block to test fragmentation impact
    void *large_block = malloc(16000); // Adjust size as needed
    if (large_block == NULL)
    {
        printf("Failed to allocate very large block due to fragmentation\n");
    }
    else
    {
        printf("Successfully allocated very large block\n");
        free(large_block);
    }

    // Free test allocations
    for (int i = 0; i < 5; i++)
    {
        free(test_allocs[i]);
        test_allocs[i] = NULL; // Set pointer to NULL to avoid double-free
    }

    // Free remaining allocations
    for (int i = 0; i < NUM_ALLOCS; i++)
    {
        if (allocs[i] != NULL)
        {
            free(allocs[i]);
            allocs[i] = NULL; // Set pointer to NULL
        }
    }

    return 0;
}