#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATH_LENGTH 256
#define MAX_COMM_LENGTH 256

void print_process_info(const char *pid);
void list_processes();

void
print_process_info(const char *pid)
{
	char path[MAX_PATH_LENGTH];
	char comm[MAX_COMM_LENGTH];
	FILE *file;

	// Obtener el nombre del comando del proceso desde el archivo /proc/[pid]/comm
	snprintf(path, sizeof(path), "/proc/%s/comm", pid);
	file = fopen(path, "r");
	if (file == NULL) {
		perror("fopen");
		return;
	}

	if (fgets(comm, sizeof(comm), file) != NULL) {
		comm[strcspn(comm, "\n")] =
		        '\0';  // Eliminar el carácter de nueva línea al final
		printf("%s %s\n", pid, comm);
	}

	fclose(file);
}

void
list_processes()
{
	DIR *proc_dir;
	struct dirent *entry;

	proc_dir = opendir("/proc");
	if (proc_dir == NULL) {
		perror("opendir");
		return;
	}

	// Recorrer los directorios en /proc
	while ((entry = readdir(proc_dir)) != NULL) {
		// Verificar si el nombre del directorio es un número (PID)
		if (entry->d_type == DT_DIR && atoi(entry->d_name) != 0) {
			print_process_info(entry->d_name);
		}
	}

	closedir(proc_dir);
}

int
main()
{
	printf("PID COMMAND\n");
	list_processes();

	return 0;
}
