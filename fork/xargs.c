/*  gcc -Wall -pedantic xargs.c -o var
 *  seq 14 | ./var echo
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#ifndef NARGS
#define NARGS 4
#endif

void ejecutar_comando(char *args[]);
void liberar_argumentos(char *args[], int num_args);

int
main(int argc, char *argv[])
{
	(void) argc;  // Para evitar el warning de 'not used'.
	char *args[NARGS + 2] = { argv[1] };

	size_t len;
	char *lineptr = NULL;

	int i = 1;

	while ((getline(&lineptr, &len, stdin)) != -1) {
		if (i == NARGS + 1) {
			args[i] = NULL;  // Para los argumentos del execvp()
			ejecutar_comando(args);
			liberar_argumentos(args, i);
			i = 1;
		}

		lineptr[strlen(lineptr) - 1] =
		        '\0';  // Reemplazo el \n generado por getline()
		args[i] = strdup(lineptr);
		i++;
	}

	// si la cantidad de argumentos no es multiplo de NARGS.
	if (i > 1) {
		args[i] = NULL;  // Para los argumentos del execvp()
		ejecutar_comando(args);
		liberar_argumentos(args, i);
	}

	return 0;
}

/* Pre : El primer algumento de args debe ser un comando y el último NULL.
 * Post:
 * */
void
ejecutar_comando(char *args[])
{
	pid_t pidC = fork();

	if (pidC < 0) {
		perror("Error. Fork");
		exit(-1);
	}

	if (pidC == 0) {  // PROCESO HIJO
		execvp(args[0], args);
	} else {             // PROCESO PADRE
		wait(NULL);  // El padre espera a que el proceso hijo finalice.
	}
}
/*  La funcion getline() usa memoria dinamica por eso se necesita liberar la memoria.
 *  Pre : num_args numero de elementos del arreglo de argumentos.
 *  Post: Libera la memoria asignada para los elementos del arreglo de argumentos (excepto el primero).
 */
void
liberar_argumentos(char *args[], int num_args)
{
	for (int i = 1; i < num_args; i++) {
		free(args[i]);
		args[i] = NULL;
	}
}
