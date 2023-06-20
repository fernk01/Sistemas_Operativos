/* Como correr las pruebas automaticas:
 *      ./test-fork -c pingpong /home/lu/Desktop/SO/LAB-FORK/
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

int
main()
{
	int p1[2],
	        p2[2];  // arreglos para almacenar los descriptores de archivo de las tuberías

	if (pipe(p1) == -1) {
		perror("Error: pipe1");
		exit(EXIT_FAILURE);
	}

	if (pipe(p2) == -1) {
		perror("Error: pipe2");
		exit(EXIT_FAILURE);
	}
	int pid = fork();  // crear el proceso hijo

	if (pid == -1) {
		perror("fork");
		exit(1);
	}
	if (pid == 0) {  // proceso hijo
		close(p1[1]);  // cerrar el extremo de escritura de la primera tubería
		close(p2[0]);  // cerrar el extremo de lectura de la segunda tubería

		int val;  // variable para almacenar el valor recibido y enviado

		// leer un valor desde la primera tubería
		if (read(p1[0], &val, sizeof(int)) == -1) {
			perror("Error al leer");
			return 1;
		}


		printf("Donde fork me devuelve %d:\n", pid);
		printf(" - getpid me devuelve: %d\n", getpid());
		printf(" - getppid me devuelve: %d\n", getppid());
		printf(" - recibo valor %d vía fd=%d\n", val, p1[0]);

		// enviar el mismo valor a la segunda tubería
		if (write(p2[1], &val, sizeof(int)) == -1) {
			perror("Error al escribir");
			return 1;
		}

		printf(" - reenvío valor en fd=%d y termino\n", p2[1]);

		close(p1[0]);  // cerrar el extremo de lectura de la primera tubería
		close(p2[1]);  // cerrar el extremo de escritura de la segunda tubería

		exit(0);
	} else {               // proceso padre
		close(p1[0]);  // cerrar el extremo de lectura de la primera tubería
		close(p2[1]);  // cerrar el extremo de escritura de la segunda tubería

		srandom(time(NULL));  // inicializar el PRNG con la hora actual como semilla

		int val = random() % 10000 +
		          5000;  // generar un valor aleatorio entre 5000 y 15000

		printf("Hola, soy PID %d:\n", getpid());
		printf(" - primer pipe me devuelve: [%d, %d]\n", p1[0], p1[1]);
		printf(" - segundo pipe me devuelve: [%d, %d]\n", p2[0], p2[1]);

		printf("\nDonde fork me devuelve %d:\n", pid);
		printf(" - getpid me devuelve: %d\n", getpid());
		printf(" - getppid me devuelve: %d\n", getppid());
		printf(" - random me devuelve: %d\n", val);
		printf(" - envío valor %d a través de fd=%d\n", val, p1[1]);

		// enviar el valor a la primera tubería
		if (write(p1[1], &val, sizeof(int)) == -1) {
			perror("Error al escribir");
			return 1;
		}


		// leer un valor desde la segunda tubería
		if (read(p2[0], &val, sizeof(int)) == -1) {
			perror("Error al leer");
			return 1;
		}

		wait(NULL);  // espero a que termine el proceso hijo

		printf("\nHola, de nuevo PID % d:\n ", getpid());
		printf(" - recibi valor% d vía fd =% d \n ", val, p2[0]);

		close(p1[1]);  // cerrar el extremo de escritura del primer conducto
		close(p2[0]);  // cerrar al final del segundo conducto lectura

		exit(0);
	}
}
