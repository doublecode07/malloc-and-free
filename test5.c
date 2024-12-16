#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/resource.h>

int main()
{
    printf("Test5: Splits, Heap Growth, and Max Heap Size\n");

    // Measure start time using clock_t
    clock_t start = clock();

    // Allocate and free memory to cause splits and heap growth
    // The set of mallocs that grows heap 4 times
    char *ptr1 = malloc(1000);
    char *ptr2 = malloc(2000);
    char *ptr3 = malloc(500);
    char *ptr4 = malloc(1500);

    // Free some blocks to create free space
    free(ptr2);
    free(ptr4);

    // Allocate blocks that can fit into freed spaces (to cause splits)
    char *ptr5 = malloc(1000);
    char *ptr6 = malloc(500);

    // Free all blocks
    free(ptr1);
    free(ptr3);
    free(ptr5);
    free(ptr6);

    // Measure end time
    clock_t end = clock();

    // Calculate elapsed time in seconds
    double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Execution time: %f seconds\n", elapsed_time);

    // Measure memory usage using getrusage
    struct rusage usage;
        if (getrusage(RUSAGE_SELF, &usage) == -1)
    {
        perror("getrusage");
        exit(EXIT_FAILURE);
    }

    printf("Maximum resident set size: %ld kilobytes\n", usage.ru_maxrss);

    // Note: The custom allocator will print statistics upon exit due to atexit()

    // Expected number of splits: 2
    // Expected number of grows: 4
    // Expected max heap size: 6120 bytes

    return 0;
}