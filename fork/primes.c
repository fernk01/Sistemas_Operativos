/* El codigo funciona.
 * Como compilar:
 *      gcc -Wall -pedantic primes.c -o var
 *      ./var 35
 *
 * Pasar las pruebas:
 *      ./test-fork -c primes /home/lu/Desktop/SO/LAB-FORK/
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

void read_and_filter(int *fd);

int
main(int argc, char *argcv[])
{
	(void) argc;  // Para evitar el warning de 'not used'.
	int natural_number = atoi(argcv[1]);

	int fds[2];

	if (pipe(fds) == -1) {
		perror("Error: pipe");
		exit(EXIT_FAILURE);
	}

	for (int pidC = 2; pidC <= natural_number; pidC++) {
		if (write(fds[1], &pidC, sizeof(pidC)) < 0) {
			perror("Error en write");
			exit(-1);
		}
	}

	close(fds[1]);  // Cierro el descriptor de escritura

	read_and_filter(&fds[0]);

	return 0;
}

/* pre : fd debe contener una secuencia de números enteros.
 * post: La funcion imprimirá los numeros primos leidos. del fd.
 */
void
read_and_filter(int *fd)
{
	int fds[2];
	int pipe_error = pipe(fds);

	int pidC = fork();
	if (pidC < 0) {
		printf("Error en fork! %d\n", pidC);
		exit(-1);
	}

	if (pidC == 0) {  //  PROCESO HIJO
		int next_number;
		int prime_number;

		int read_error = read(*fd, &prime_number, sizeof(prime_number));
		if (read_error == 0) {
			exit(0);
		}
		printf("primo %d\n", prime_number);

		while (read_error != 0) {
			read_error = read(*fd, &next_number, sizeof(next_number));
			if (read_error < 0) {
				perror("Error en read");
				exit(-1);
			}

			if (next_number % prime_number != 0) {
				pipe_error = write(fds[1],
				                   &next_number,
				                   sizeof(next_number));
				if (pipe_error < 0) {
					perror("Error en write");
					exit(-1);
				}
			}
		}

		close(fds[1]);  // Cierro el de escritura.
		close(*fd);

		read_and_filter(
		        &fds[0]);  // Envio recursivamente los nuevos numeros primos.
		exit(0);
	} else {  // PROCESO PADRE
		close(*fd);
		close(fds[0]);
		close(fds[1]);

		wait(NULL);  // Espera a que el proceso hijo finalice.
	}
	exit(0);
}
