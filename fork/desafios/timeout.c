/*
 *
 *   gcc -ggdb3 -O2 -Wall -Wextra -std=c11 -Wmissing-prototypes -Wvla
 * -D_DEFAULT_SOURCE timeout.c -o timeout
 *  ./timeout 5 ls -l
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int
main(int argc, char *argv[])
{
	if (argc < 3) {
		printf("Uso: %s duración comando [argumentos...]\n", argv[0]);
		return 1;
	}

	int duration = atoi(argv[1]);
	if (duration <= 0) {
		printf("Duración inválida\n");
		return 1;
	}

	// Crear el temporizador
	timer_t timerid;
	struct sigevent event;
	struct itimerspec timer_spec;
	memset(&event, 0, sizeof(struct sigevent));
	memset(&timer_spec, 0, sizeof(struct itimerspec));

	event.sigev_notify = SIGEV_SIGNAL;
	event.sigev_signo = SIGTERM;

	if (timer_create(CLOCK_REALTIME, &event, &timerid) == -1) {
		perror("Error al crear el temporizador");
		return 1;
	}

	// Establecer la duración del temporizador
	timer_spec.it_value.tv_sec = duration;

	if (timer_settime(timerid, 0, &timer_spec, NULL) == -1) {
		perror("Error al establecer el temporizador");
		timer_delete(timerid);
		return 1;
	}

	// Ejecutar el comando
	pid_t pid = fork();
	if (pid == -1) {
		perror("Error al crear el proceso hijo");
		timer_delete(timerid);
		return 1;
	} else if (pid == 0) {
		// Proceso hijo
		execvp(argv[2], &argv[2]);
		perror("Error al ejecutar el comando");
		exit(1);
	} else {
		// Proceso padre
		int status;
		if (waitpid(pid, &status, 0) == -1) {
			perror("Error al esperar al proceso hijo");
			timer_delete(timerid);
			return 1;
		}

		// Cancelar el temporizador si el proceso termina antes de tiempo
		struct itimerspec zero_spec;
		memset(&zero_spec, 0, sizeof(struct itimerspec));

		if (timer_settime(timerid, 0, &zero_spec, NULL) == -1) {
			perror("Error al cancelar el temporizador");
			timer_delete(timerid);
			return 1;
		}

		// Verificar si el proceso hijo fue terminado por el temporizador
		if (WIFSIGNALED(status) && WTERMSIG(status) == SIGTERM) {
			printf("El comando excedió la duración especificada y "
			       "fue terminado.\n");
		}
	}

	timer_delete(timerid);
	return 0;
}
