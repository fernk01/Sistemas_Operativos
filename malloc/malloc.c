#define _DEFAULT_SOURCE

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#include "malloc.h"
#include "printfmt.h"

#define ALIGN4(s) (((((s) -1) >> 2) << 2) + 4)
#define REGION2PTR(r) ((r) + 1)
#define PTR2REGION(ptr) ((struct region *) (ptr) -1)

struct region {
	bool free;
	size_t size;          // tamaño de la región
	struct region *next;  // siguiente región
	struct region *prev;  // región anterior
};

const size_t SIZE_HEADER = sizeof(struct region);

// Punteros a las listas de bloques.
struct region *small_blocks[MAX_BLOCKS] = { NULL };
struct region *medium_blocks[MAX_BLOCKS] = { NULL };
struct region *large_blocks[MAX_BLOCKS] = { NULL };

int amount_of_mallocs = 0;  // cantidad de mallocs.
int amount_of_frees = 0;    // cantidad de frees.
int requested_memory = 0;   // cantidad de memoria pedida.

// Devuelve la lista de bloques correspondiente al tamaño.
struct region **
get_blocks_by_size(size_t size)
{
	if (size <= SMALL_BLOCKS) {
		return small_blocks;
	} else if (size <= MEDIUM_BLOCKS) {
		return medium_blocks;
	} else if (size <= LARGE_BLOCKS) {
		return large_blocks;
	}

	return NULL;
}

// Devuelve el tamaño del bloque correspondiente al tamaño.
size_t
get_block_size(size_t size)
{
	if (size < SMALL_BLOCKS) {
		return SMALL_BLOCKS;
	} else if (size < MEDIUM_BLOCKS) {
		return MEDIUM_BLOCKS;
	} else if (size < LARGE_BLOCKS) {
		return LARGE_BLOCKS;
	}

	return 0;
}

// finds the next free region
// that holds the requested size
static struct region *
find_free_region(size_t size)
{
	struct region **block_list = get_blocks_by_size(size + SIZE_HEADER);

	struct region *current_region = NULL;
	size_t i = 0;

#ifdef FIRST_FIT
	// Your code here for "first fit"
	while (block_list[i] != NULL && i < MAX_BLOCKS) {
		current_region = block_list[i];

		// Recorro región por región hasta encontrar una libre.
		while (current_region != NULL) {
			if (current_region->free) {
				// Spliting.
				if (current_region->size > size) {
					return create_region(current_region, size);
				} else {  // Coalescing.

					// Verifica si la region siguiente.
					if (current_region->next != NULL &&
					    current_region->next->free &&
					    current_region->size +
					                    current_region->next->size +
					                    sizeof(struct region) >=
					            size) {
						return create_region(
						        coalesce_right(
						                current_region),
						        size);
					}
					// Verifica si se pueden unir tres regiones.
					if (current_region->next != NULL &&
					    current_region->prev != NULL &&
					    current_region->next->free &&
					    current_region->prev->free &&
					    current_region->size +
					                    current_region->next->size +
					                    current_region->prev->size +
					                    2 * sizeof(struct region) >=
					            size) {
						coalesce_right(current_region);
						return create_region(
						        coalesce_right(
						                current_region->prev),
						        size);
					}
				}
			}
			current_region = current_region->next;
		}

		i++;
	}


#endif

#ifdef BEST_FIT

	struct region *best_fit_region = NULL;
	size_t best_fit_size = get_block_size(size);

	while (block_list[i] != NULL && i < MAX_BLOCKS) {
		current_region = block_list[i];

		// Recorro región por región hasta encontrar una libre.
		while (current_region != NULL) {
			if (current_region->free) {
				// Si el tamaño de la región es igual al tamaño solicitado, no necesitamos buscar más.
				if (current_region->size == size) {
					return current_region;
				}
				// Si el tamaño de la región es mayor al tamaño
				// solicitado y menor que el mejor ajuste actual, actualizamos la mejor opción.
				if (current_region->size > size &&
				    current_region->size < best_fit_size) {
					best_fit_region = current_region;
					best_fit_size = current_region->size;
				}
			}
			current_region = current_region->next;
		}
		if (best_fit_region != NULL) {
			return create_region(best_fit_region, size);
		}

		i++;
	}

	return best_fit_region;

#endif

	return current_region;
}

// Une la region que recibe como parametro con la region siguiente.
struct region *
coalesce_right(struct region *region)
{
	if (region->next != NULL && region->next->free) {
		region->size += region->next->size + sizeof(struct region);
		region->next = region->next->next;
		if (region->next != NULL) {
			region->next->prev = region;
		}
	}
	return region;
}

// Crea un bloque: header + memoria.
struct region *
create_block(size_t size)
{
	void *block = mmap(NULL,
	                   size,
	                   PROT_READ | PROT_WRITE,
	                   MAP_ANONYMOUS | MAP_PRIVATE,
	                   0,
	                   0);

	if (block == MAP_FAILED) {
		errno = ENOMEM;
		return NULL;
	}

	struct region *new_region = (struct region *) block;
	new_region->size = size - sizeof(struct region);
	new_region->free = false;
	new_region->next = NULL;
	new_region->prev = NULL;

	return new_region;
}

// Spliting. Crea una región de memoria de tamaño: header + size.
struct region *
create_region(struct region *region, size_t size)
{
	if (size > region->size) {
		return NULL;
	}

	struct region *new_region = (void *) region + size;
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

/// Public API of malloc library ///
void *
malloc(size_t size)
{
	if (size <= 0) {
		return NULL;
	}

	if (size > LARGE_BLOCKS) {
		errno = ENOMEM;
		return NULL;
	}

	// Alinea el tamaño a múltiplos de 4 bytes.
	size = ALIGN4(size);
	if (size < MIN_SIZE) {
		size = MIN_SIZE;
	}

	struct region *free_region = find_free_region(size);

	// Si no hay una región libre, se crea un nuevo bloque de memoria.
	struct region **current_block = NULL;
	struct region *new_block = NULL;
	size_t size_block;
	size_t i = 0;

	if (free_region == NULL) {
		current_block = get_blocks_by_size(size + SIZE_HEADER);
		size_block = get_block_size(size);

		while (*current_block != NULL && i < MAX_BLOCKS) {
			current_block++;
			i++;
		}

		if (i == MAX_BLOCKS)
			return NULL;

		new_block = create_block(size_block);
		free_region = create_region(new_block, size);

		*current_block = free_region;
	}

	amount_of_mallocs++;
	requested_memory += size;

	return REGION2PTR(free_region);
}

void
free(void *ptr)
{
	if (ptr == NULL) {
		return;
	}

	struct region *curr = PTR2REGION(ptr);

	// Verifico que el puntero sea valido, es decir, que sea un puntero a una region.
	struct region **block_list = get_blocks_by_size(curr->size);
	struct region *current_region;
	size_t i = 0;

	for (i = 0; i < MAX_BLOCKS; i++) {
		current_region = block_list[i];
		while (current_region != NULL && current_region != curr) {
			current_region = current_region->next;
		}

		if (current_region == curr) {
			break;
		}
	}

	// No se encontro el puntero en la lista de bloques.
	if (current_region == NULL) {
		return;
	}

	// Verificamos si las regiones contigua a derecha-izquierda estan libres.
	// Si es asi, une las regiones contiguas. Verifica si el bloque
	// de memoria esta vacio, si es asi, se libera el bloque de memoria con munmap.
	if (curr->next != NULL && curr->next->free) {
		curr->free = true;
		curr = coalesce_right(curr);
	}
	if (curr->prev != NULL && curr->prev->free) {
		curr->free = true;
		curr = coalesce_right(curr->prev);
	}
	if (curr->next == NULL && curr->prev == NULL) {
		if (munmap(curr, curr->size + sizeof(struct region)) == -1)
			return;

		block_list[i] = NULL;
		amount_of_frees++;
	} else {
		if (curr->free == false)
			amount_of_frees++;

		curr->free = true;
	}
}

void *
calloc(size_t nmemb, size_t size)
{
	if (nmemb == 0 || size == 0) {
		return NULL;
	}
	if (nmemb * size > LARGE_BLOCKS) {
		errno = ENOMEM;
		return NULL;
	}

	void *ptr = malloc(nmemb * size);
	if (ptr != NULL) {
		memset(ptr, 0, nmemb * size);
		requested_memory += nmemb * size;
	}

	return ptr;
}

void *
realloc(void *ptr, size_t size)
{
	// Si es NULL, realloc se comporta como malloc y asigna un nuevo bloque de memoria.
	if (ptr == NULL) {
		return malloc(size);
	}

	// Si es 0, realloc se comporta como free y libera el bloque de memoria apuntado por ptr.
	if (size == 0) {
		free(ptr);
		return NULL;
	}

	size = ALIGN4(size);
	if (size < MIN_SIZE) {
		size = MIN_SIZE;
	}

	if (size > LARGE_BLOCKS) {
		errno = ENOMEM;
		return NULL;
	}

	struct region *region = PTR2REGION(ptr);

	if (region->size >= size) {  // Achicar la región.
		region = create_region(region, size);
		requested_memory += size;

		return REGION2PTR(region);
	} else {  // Agrandar la región.
		// Verifico si la memoria contigua a la derecha esta libre y si la memoria es mayor a size.
		if (region->next != NULL && region->next->free &&
		    region->size + region->next->size + sizeof(struct region) >=
		            size) {
			region = create_region(coalesce_right(region), size);
			requested_memory += size;

			return REGION2PTR(region);
		} else {
			// Si no es asi, se crea un nuevo bloque de memoria y se copia el contenido del bloque anterior.
			void *new_ptr = malloc(size);
			if (new_ptr == NULL) {
				errno = ENOMEM;
				return NULL;
			}

			memcpy(new_ptr, ptr, region->size);
			free(ptr);

			requested_memory += size;

			return new_ptr;
		}
	}

	return NULL;
}

void
get_stats(struct malloc_stats *stats)
{
	stats->mallocs = amount_of_mallocs;
	stats->frees = amount_of_frees;
	stats->requested_memory = requested_memory;
}
