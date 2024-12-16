#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main()
{
    printf("test7: malloc_homework.pdf reference");

    unsigned char* ptr1 = NULL;
    unsigned char* ptr2 = NULL;
    unsigned char* ptr3 = NULL;
    unsigned char* ptr4 = NULL;
    ptr1 = malloc(2048);
    ptr2 = malloc(1024);
    ptr3 = malloc(4096);
    free(ptr2);
    ptr4 = malloc(512);
    free(ptr4);

    memset(ptr1, 0, 2048);
    memset(ptr3, 0, 4096);

    return 0;
}