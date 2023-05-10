#define _DEFAULT_SOURCE

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>	// mmap(2)
#include <errno.h>

#include "malloc.h"
#include "printfmt.h"

#define ALIGN4(s) (((((s) -1) >> 2) << 2) + 4)
#define REGION2PTR(r) ((r) + 1)
#define PTR2REGION(ptr) ((struct region *) (ptr) -1)

struct region {
	bool free;
	size_t size;	// tamaño de la región
	struct region *next;	// siguiente región
	struct region *prev;	// región anterior
};

struct region *region_free_list = NULL;

// Punteros a las listas de bloques.
struct region *small_blocks[MAX_BLOCKS] = {NULL};
struct region *medium_blocks[MAX_BLOCKS] = {NULL};
struct region *large_blocks[MAX_BLOCKS] = {NULL};


int amount_of_mallocs = 0; // cantidad de mallocs.
int amount_of_frees = 0; // cantidad de frees.
int requested_memory = 0; // cantidad de memoria pedida.

// finds the next free region
// that holds the requested size
static struct region *
find_free_region(size_t size)
{
	// struct region *next = region_free_list;

	struct region **block_list = NULL;

	if (size + sizeof(struct region) < SMALL_BLOCKS)
		block_list = small_blocks;
	else if (size + sizeof(struct region) < MEDIUM_BLOCKS)
		block_list = medium_blocks;
	else if (size + sizeof(struct region) < LARGE_BLOCKS)
		block_list = large_blocks;
	
	struct region *current_region = NULL;
	size_t i = 0;

#ifdef FIRST_FIT
	// Your code here for "first fit"
	while(block_list[i] != NULL && i < MAX_BLOCKS) {
		current_region = block_list[i];

		// Recorro región por región hasta encontrar una libre.
		while (current_region != NULL) {
			if (current_region->free) {
				// Spliting.
				if (current_region->size + sizeof(struct region) >= size) {
					return create_region(current_region, size);
				}
				else { // Coalescing.
					
					// Verifica si la region siguiente.
					if (current_region->next != NULL && current_region->next->free && current_region->size + current_region->next->size + sizeof(struct region) >= size) {
						return create_region(coalesce_right(current_region), size);
					}
					// Verifica si se pueden unir tres regiones.
					if (current_region->next != NULL && current_region->prev != NULL && current_region->next->free && current_region->prev->free && current_region->size + current_region->next->size + current_region->prev->size + 2*sizeof(struct region) >= size) {
						coalesce_right(current_region);
						return create_region(coalesce_right(current_region->prev), size);
					}
						
				}
			}	
				current_region = current_region->next;
		}
		
		i++;
	}
			
	
#endif

#ifdef BEST_FIT
	// Your code here for "best fit"
#endif

	return current_region;
}

// Creamos una funcion para unir dos regiones de memoria contiguas a derecha.
struct region *coalesce_right(struct region *region) {
	if (region->next != NULL && region->next->free) {
		region->size += region->next->size + sizeof(struct region);
		region->next = region->next->next;
		if (region->next->next != NULL)
			region->next->next->prev = region;
	}
	return region;
}

/* Crea un bloque de memoria de tamaño size. */
struct region *create_block(size_t size) {
    void *block = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);

    if (block == MAP_FAILED) {
        return NULL;
    }

    struct region *new_region = (struct region *) block;
    new_region->size = size - sizeof(struct region);
    new_region->free = false;
    new_region->next = NULL;
	new_region->prev = NULL;

    return new_region;
}
// Spliting. Crea una región de memoria de tamaño header+size.
struct region *create_region(struct region *region, size_t size) {
	if (size > region->size) {
		return NULL;
	}

	struct region *new_region = (void *)region + size;
	new_region->size = region->size - size - sizeof(struct region);
	new_region->free = true;
	new_region->next = region->next;
	new_region->prev = region;
    
    if (region->next != NULL) {
        region->next->prev = new_region;
    }

	region->size = size;
	region->free = false;
	region->next = new_region;

	return region;
}

// Retorna la lista de bloques en la que se encuentra el tamaño size.
struct region **list_blocks(size_t size) {
	if (size < SMALL_BLOCKS)
		return small_blocks;
	else if (size < MEDIUM_BLOCKS)
		return medium_blocks;
	else if (size < LARGE_BLOCKS)
		return large_blocks;

	return NULL;
}

/// Public API of malloc library ///

void *
malloc(size_t size)
{
	if (size <= 0) {
		return NULL;
	}

	if (requested_memory + size > LIMIT_MEMORY) {
		errno = ENOMEM;
		return NULL; 
	}

	// Alinea el tamaño a múltiplos de 4 bytes.
	size = ALIGN4(size);
	if (size < MIN_SIZE) {
		size = MIN_SIZE;
	}

	struct region *free_region = find_free_region(size);
	struct region *new_block = NULL;

	struct region **current_block = NULL;
	size_t size_block;

	// Si no hay una región libre, se crea un nuevo bloque de memoria.
	if (free_region == NULL) {
		if (size + sizeof(struct region) < SMALL_BLOCKS) {
			size_block = SMALL_BLOCKS;
			current_block = small_blocks;
		}
		else if (size + sizeof(struct region) < MEDIUM_BLOCKS) {
			size_block = MEDIUM_BLOCKS;
			current_block =  medium_blocks;
		}
		else if (size + sizeof(struct region) < LARGE_BLOCKS) {
			size_block = LARGE_BLOCKS;
			current_block = large_blocks;
		}

		size_t i = 0;
		while (*current_block != NULL && i < MAX_BLOCKS) {
			current_block++;
		}

		if(i == MAX_BLOCKS)
			return NULL;

		new_block = create_block(size_block);
		free_region = create_region(new_block, size);

		*current_block = new_block;
	}
	
	amount_of_mallocs++;
	requested_memory += size;

	return REGION2PTR(free_region);
}

void
free(void *ptr)
{
	// updates statistics
	amount_of_frees++;

	struct region *curr = PTR2REGION(ptr);
	assert(curr->free == 0);

	curr->free = true;

	// Your code here
	//
	// hint: maybe coalesce regions?
}

void *
calloc(size_t nmemb, size_t size)
{
	// Your code here

	return NULL;
}

void *
realloc(void *ptr, size_t size)
{
	// Your code here

	return NULL;
}

void
get_stats(struct malloc_stats *stats)
{
	stats->mallocs = amount_of_mallocs;
	stats->frees = amount_of_frees;
	stats->requested_memory = requested_memory;
}
