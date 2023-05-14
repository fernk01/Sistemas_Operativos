#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "testlib.h"
#include "malloc.h"

void prueba_malloc_casos_borde(void);

void prueba_free(void);
void prueba_calloc(void);
void prueba_realloc(void);

void prueba_first_fit(void);
void prueba_best_fit(void);

void prueba_bloque_chico(void);
void prueba_bloque_mediano(void);
void prueba_bloque_grande(void);

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

	ASSERT_TRUE("-successful malloc returns non null pointer", var != NULL);

	free(var);
}

static void
correct_copied_value(void)
{
	char *test_string = "FISOP malloc is working!";

	char *var = malloc(100);

	strcpy(var, test_string);

	ASSERT_TRUE("-allocated memory should contain the copied value",
	            strcmp(var, test_string) == 0);

	free(var);
}

static void
correct_amount_of_mallocs(void)
{
	struct malloc_stats stats;

	char *var = malloc(100);

	free(var);
	printfmt("-mallocs: ");
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

	ASSERT_TRUE("-amount of frees should be one", stats.frees == 1);
}

static void
correct_amount_of_requested_memory(void)
{
	struct malloc_stats stats;

	char *var = malloc(100);

	free(var);

	get_stats(&stats);

	ASSERT_TRUE("-amount of requested memory should be 100",
	            stats.requested_memory == 100);
}

void
prueba_malloc_casos_borde(void)
{
	char *var = malloc(0);
	ASSERT_TRUE("-malloc(0) debe retornar NULL", var == NULL);
	free(var);

	char *var2 = malloc(100);
	ASSERT_TRUE("-malloc(100) debe retornar un puntero no nulo", var2 != NULL);
	free(var2);

	char *var3 = malloc(1000000000);
	ASSERT_TRUE("-malloc(1000000000) debe retornar NULL", var3 == NULL);
	free(var3);

	// Pedido de memoria negativo.
	int num = -1;
	char *var4 = malloc(num);
	ASSERT_TRUE("-malloc(-1) debe retornar NULL", var4 == NULL);
	free(var4);
}

void
prueba_free(void)
{
	struct malloc_stats stats;

	free(NULL);

	get_stats(&stats);

	ASSERT_TRUE("-free(NULL) no devuelve nada.",
	            stats.requested_memory == 0 && stats.frees == 0);

	// Prueba: Liberar memoria que no fue pedida.
	char *var = malloc(200); /*
	 free(var + 220);
	 get_stats(&stats);
	 ASSERT_TRUE("Liberar memoria que no fue pedida no modifica las
	 estadisticas", (stats.frees == 0) && (stats.requested_memory == 200));
	 */
	// Prueba: Liberar correctamente.
	free(var);
	get_stats(&stats);
	ASSERT_TRUE(
	        "-free(var) correcto funcionamiento, modifica las estadisticas",
	        stats.frees == 1);
}

void
prueba_calloc(void)
{
	// calloc con tamanio 0 devuelve null:
	char *var = calloc(0, 100);
	ASSERT_TRUE("-calloc(0, 100) debe retornar NULL", var == NULL);

	// calloc con tamanio negativo devuelve null:
	size_t num = -1;
	char *var2 = calloc(num, 100);
	ASSERT_TRUE("-calloc(-1, 100) debe retornar NULL", var2 == NULL);

	// calloc funcionando correctamente.
	char *var3 = calloc(1500, sizeof(char));
	ASSERT_TRUE(
	        "-calloc(1500, sizeof(char)) debe retornar un puntero no nulo",
	        var3 != NULL);
	free(var3);

	// Prueba calloc inicizando la memoria en 0.
	char *var4 = calloc(1500, sizeof(char));
	char str[1500] = { 0 };
	ASSERT_TRUE(
	        "-calloc(1500, sizeof(char)) inicializa la memoria en ceros",
	        memcmp(var4, str, 1500) == 0);
	free(var4);
}

void
prueba_realloc(void)
{
	struct malloc_stats stats;

	char *var = realloc(NULL, 0);
	ASSERT_TRUE("-realloc con puntero a NULL y tamaño 0 devuelve NULL",
	            var == NULL);

	int num = -1;
	char *var2 = realloc(NULL, num);
	ASSERT_TRUE(
	        "-realloc con puntero a NULL y tamaño negativo devuelve NULL",
	        var == NULL);

	char *var3 = realloc(NULL, 200);
	get_stats(&stats);
	ASSERT_TRUE("-realloc con puntero NULL se comporta como malloc",
	            stats.mallocs == 1);
	ASSERT_TRUE("-realloc con puntero NULL aloca memoria correctamente",
	            stats.requested_memory == 200);

	free(var3);

	int prev_mem_requested = stats.requested_memory;
	char *var4 = malloc(1000);
	var4 = realloc(var4, 2000);
	get_stats(&stats);
	ASSERT_TRUE("-realloc agranda bloque previamente reservado",
	            stats.requested_memory == 3000 + prev_mem_requested);

	free(var);
	free(var2);
	free(var4);
}

void
prueba_first_fit(void)
{
	// Prueba de asignación de un bloque de memoria en medio de dos bloques
	// ocupados: Crea dos bloques de memoria contiguos y ocupa parte del espacio
	// en uno de ellos. Luego, solicita un nuevo bloque de memoria y verifica
	// que se asigne en el espacio libre existente entre los dos bloques ocupados.
	int *region = malloc(1500);
	int *region2 = malloc(5400);
	int *region3 = malloc(6800);

	free(region2);

	int *region4 = malloc(2300);

	ASSERT_TRUE("-Prueba: eleccion primer region de memoria.",
	            region4 == region2);

	free(region);
	free(region3);
	free(region4);
}

void
prueba_best_fit(void)
{
	int *region1 = malloc(5000);

	int *region2 = malloc(3000);
	int *region3 = malloc(4500);

	free(region2);

	int *region4 = malloc(2600);
	int *region5 = malloc(7500);

	free(region4);

	int *region6 = malloc(2650);

	ASSERT_TRUE("-Prueba: eleccion de la mejor region de memoria.",
	            region6 == region4);

	free(region1);
	free(region3);
	free(region5);
}

void
prueba_bloque_chico(void)
{
	struct malloc_stats stats;
	int block_chico = 9500;
	int *var = malloc(block_chico);
	get_stats(&stats);
	ASSERT_TRUE("-Malloc de un tamaño chico creado correctamente",
	            stats.mallocs == 1);
	ASSERT_TRUE("-Malloc de un tamaño chico reserva memoria correctamente",
	            stats.requested_memory == block_chico);
	free(var);

	int *var1 = malloc(block_chico);
	int *var2 = malloc(block_chico);
	int *var3 = malloc(block_chico);
	int *var4 = malloc(block_chico);
	int *var5 = malloc(block_chico);
	int mem_total = 5 * block_chico;
	get_stats(&stats);
	ASSERT_TRUE("-Mallocs de un tamaño chico creado correctamente",
	            stats.mallocs == 6);
	ASSERT_TRUE("-Malloc de un tamaño chico reserva memoria correctamente",
	            stats.requested_memory == mem_total + block_chico);

	free(var1);
	free(var2);
	free(var3);
	free(var4);
	free(var5);
}

void
prueba_bloque_mediano(void)
{
	struct malloc_stats stats;
	int block_mediano = 1600500;
	int *var = malloc(block_mediano);
	get_stats(&stats);
	ASSERT_TRUE("-Malloc de un tamaño mediano creado correctamente",
	            stats.mallocs == 1);
	ASSERT_TRUE(
	        "-Malloc de un tamaño mediano reserva memoria correctamente",
	        stats.requested_memory == block_mediano);
	free(var);

	int *var1 = malloc(block_mediano);
	int *var2 = malloc(block_mediano);
	int *var3 = malloc(block_mediano);
	int *var4 = malloc(block_mediano);
	int *var5 = malloc(block_mediano);
	int mem_total = 5 * block_mediano;
	get_stats(&stats);
	ASSERT_TRUE("-Mallocs de un tamaño mediano creado correctamente",
	            stats.mallocs == 6);
	ASSERT_TRUE(
	        "-Malloc de un tamaño mediano reserva memoria correctamente",
	        stats.requested_memory == mem_total + block_mediano);

	free(var1);
	free(var2);
	free(var3);
	free(var4);
	free(var5);
}

void
prueba_bloque_grande(void)
{
	struct malloc_stats stats;
	int block_grande = 10020400;
	int *var = malloc(block_grande);

	get_stats(&stats);

	ASSERT_TRUE("-Malloc de un tamaño grande creado correctamente",
	            stats.mallocs == 1);
	ASSERT_TRUE("-Malloc de un tamaño grande reserva memoria correctamente",
	            stats.requested_memory == block_grande);
	free(var);

	int *var1 = malloc(block_grande);
	int *var2 = malloc(block_grande);
	int *var3 = malloc(block_grande);
	int *var4 = malloc(block_grande);
	int *var5 = malloc(block_grande);

	int mem_total = 5 * block_grande;

	get_stats(&stats);

	ASSERT_TRUE("-Mallocs de un tamaño grande creado correctamente",
	            stats.mallocs == 6);
	ASSERT_TRUE("-Malloc de un tamaño grande reserva memoria correctamente",
	            stats.requested_memory == mem_total + 10020400);

	free(var1);
	free(var2);
	free(var3);
	free(var4);
	free(var5);
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

	printfmt("\nPRUEBAS REALLOC:\n");
	run_test(prueba_realloc);

#ifdef FIRST_FIT
	printfmt("\nPRUEBAS FIRST-FIT:\n");
	run_test(prueba_first_fit);
#endif

#ifdef BEST_FIT
	printfmt("\nPRUEBAS BEST-FIT:\n");
	run_test(prueba_best_fit);
#endif

	printfmt("\nPRUEBAS BLOQUE CHICO:\n");
	run_test(prueba_bloque_chico);

	printfmt("\nPRUEBAS BLOQUE MEDIANO:\n");
	run_test(prueba_bloque_mediano);

	printfmt("\nPRUEBAS BLOQUE GRANDE:\n");
	run_test(prueba_bloque_grande);

	return 0;
}
