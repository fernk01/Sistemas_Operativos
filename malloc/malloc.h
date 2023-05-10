#ifndef _MALLOC_H_
#define _MALLOC_H_


#define MIN_SIZE 256
#define SMALL_BLOCKS 16384	// en bytes
#define MEDIUM_BLOCKS 1048576
#define LARGE_BLOCKS 33554432

#define MAX_BLOCKS 100
#define LIMIT_MEMORY (MAX_BLOCKS * (SMALL_BLOCKS + MEDIUM_BLOCKS + LARGE_BLOCKS))	

struct malloc_stats {
	int mallocs;
	int frees;
	int requested_memory;
};

void *malloc(size_t size);

void free(void *ptr);

void *calloc(size_t nmemb, size_t size);

void *realloc(void *ptr, size_t size);

void get_stats(struct malloc_stats *stats);

#endif  // _MALLOC_H_
