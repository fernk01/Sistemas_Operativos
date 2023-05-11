#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "testlib.h"
#include "malloc.h"

void prueba_malloc_casos_borde(void);
void prueba_free(void);
void prueba_calloc(void);
void prueba_bloque_pequenio(void);

struct region {
	bool free;
	size_t size;          // tamaño de la región
	struct region *next;  // siguiente región
	struct region *prev;  // región anterior
};

static void
successful_malloc_returns_non_null_pointer(void)
{
	char *var = malloc(100);

	ASSERT_TRUE("successful malloc returns non null pointer", var != NULL);

	free(var);
}

static void
correct_copied_value(void)
{
	char *test_string = "FISOP malloc is working!";

	char *var = malloc(100);

	strcpy(var, test_string);

	ASSERT_TRUE("allocated memory should contain the copied value",
	            strcmp(var, test_string) == 0);

	free(var);
}

static void
correct_amount_of_mallocs(void)
{
	struct malloc_stats stats;

	char *var = malloc(100);

	free(var);
	printfmt("mallocs: ");
	get_stats(&stats);


	ASSERT_TRUE("amount of mallocs should be one", stats.mallocs == 1);
}

static void
correct_amount_of_frees(void)
{
	struct malloc_stats stats;

	char *var = malloc(100);

	free(var);

	get_stats(&stats);

	ASSERT_TRUE("amount of frees should be one", stats.frees == 1);
}

static void
correct_amount_of_requested_memory(void)
{
	struct malloc_stats stats;

	char *var = malloc(100);

	free(var);

	get_stats(&stats);

	ASSERT_TRUE("amount of requested memory should be 100",
	            stats.requested_memory == 100);
}

void
prueba_malloc_casos_borde(void)
{
	char *var = malloc(0);
	ASSERT_TRUE("malloc(0) debe retornar NULL", var == NULL);
	free(var);

	char *var2 = malloc(100);
	ASSERT_TRUE("malloc(100) debe retornar un puntero no nulo", var2 != NULL);
	free(var2);

	char *var3 = malloc(1000000000);
	ASSERT_TRUE("malloc(1000000000) debe retornar NULL", var3 == NULL);
	free(var3);

	// Pedido de memoria negativo.
	int num = -1;
	char *var4 = malloc(num);
	ASSERT_TRUE("malloc(-1) debe retornar NULL", var4 == NULL);
	free(var4);
}
void
prueba_free(void)
{
	struct malloc_stats stats;

	free(NULL);

	get_stats(&stats);

	ASSERT_TRUE("free(NULL) no debe modifica amount_of_frees ",
	            stats.frees == 0);
	ASSERT_TRUE("free(NULL) no debe modifica requested_memory ",
	            stats.requested_memory == 0);
}
void
prueba_calloc(void)
{
	// calloc con tamanio 0 devuelve null:
	char *var = calloc(0, 100);
	ASSERT_TRUE("calloc(0, 100) debe retornar NULL", var == NULL);
}
void
prueba_bloque_pequenio(void)
{
	// Primer Bloque de mamoria.
	// Se crean dos regiones de memoria contiguas de tamaño 1500 y 10600. La
	// tercera region debe ser de tamaño 4188 y estar libre.
	int *region = malloc(1500);
	int *region2 = malloc(10600);
	struct region *aux = ((struct region *) (region2) -1);
	int tamano = aux->next->size;
	printfmt("Se crean dos regiones de memoria contiguas de tamaño 1500 y "
	         "10600. La tercera region debe ser de tamaño 4188 y estar "
	         "libre.\n");
	ASSERT_TRUE("la region contigua a region2 debe ser de tamaño 4188",
	            tamano == 4188);
	ASSERT_TRUE("la region contigua a region2 debe estar libre",
	            aux->next->free == true);
}

int
main(void)
{
	printfmt("PRUEBAS CATEDRA:\n");
	run_test(successful_malloc_returns_non_null_pointer);
	run_test(correct_copied_value);
	run_test(correct_amount_of_mallocs);
	run_test(correct_amount_of_frees);
	run_test(correct_amount_of_requested_memory);

	printfmt("\nPRUEBAS MALLOC CASOS BORDES:\n");
	run_test(prueba_malloc_casos_borde);

	printfmt("\nPRUEBAS FREE:\n");
	run_test(prueba_free);

	printfmt("\nPRUEBAS CALLOC:\n");
	run_test(prueba_calloc);

	printfmt("\nPRUEBAS BLOQUE PEQUEÑOS:\n");
	run_test(prueba_bloque_pequenio);

	return 0;
}
