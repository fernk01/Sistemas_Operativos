/* Como ejecutar:
 *  ./test-fork -c find /home/lu/Desktop/SO/LAB-FORK
 */
#define _GNU_SOURCE  // strcasestr()

#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define PATH_MAX 4096

void search_file_directory(DIR *dirp,
                           char *dir_path,
                           char *find,
                           char *(*str_func)(const char *, const char *) );

/* Pre :  El parámetro dirp debe ser un puntero DIR válido que apunte a un
 * directorio abierto. dir_path debe ser una cadena de caracteres que contenga
 * la ruta del directorio que se está buscando. find debe ser una cadena de
 * caracteres que contenga la cadena de búsqueda. str_func debe ser un puntero
 * válido a una función que tome dos cadenas de caracteres como entrada y
 * devuelva un puntero a una cadena de caracteres. Post:  La función busca en el
 * directorio actual y en todos sus subdirectorios para encontrar archivos que
 * contengan la cadena de búsqueda y los imprime en la consola. La ruta del
 * directorio (parámetro dir_path) puede haber sido modificada, pero vuelve a su
 * valor original una vez que se sale del subdirectorio
 */
void
search_file_directory(DIR *dirp,
                      char *dir_path,
                      char *find,
                      char *(*str_func)(const char *, const char *) )
{
	struct dirent *entry;
	char path_buf[PATH_MAX];  // Necesario para no perder el path inicial

	strcpy(path_buf, dir_path);

	while ((entry = readdir(dirp)) != NULL) {
		if (str_func(entry->d_name, find) != NULL) {
			if ((strlen(dir_path) == 1)) {
				printf("%s\n", entry->d_name);
			} else {
				printf("%s/%s\n", dir_path + 2, entry->d_name);
			}
		}
		// Si es un directorio y distinto del directorio '.' y '..'.
		if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 &&
		    strcmp(entry->d_name, "..") != 0) {
			int new_fd =
			        openat(dirfd(dirp), entry->d_name, O_DIRECTORY);
			if (new_fd == -1) {
				perror("Error: openat().");
				exit(-1);
			}
			strcat(dir_path, "/");
			strcat(dir_path, entry->d_name);

			DIR *subdirp = fdopendir(new_fd);
			search_file_directory(subdirp, dir_path, find, str_func);
			closedir(subdirp);  // Cierra el subdirectorio abierto
			dir_path = path_buf;
		}
	}
}

int
main(int argc, char *argv[])
{
	char dir_path[PATH_MAX] = ".";
	DIR *dirp = opendir(dir_path);

	if (dirp == NULL) {
		perror("Error al abrir el directorio");
		exit(-1);
	}

	if (argc == 2) {
		search_file_directory(dirp, dir_path, argv[1], strstr);
	} else if (argc == 3 && strcmp(argv[1], "-i") == 0) {
		search_file_directory(dirp, dir_path, argv[2], strcasestr);
	} else {
		fprintf(stderr, "Uso: %s [-i] cadena_a_buscar\n", argv[0]);
		exit(-1);
	}

	closedir(dirp);
	return 0;
}
