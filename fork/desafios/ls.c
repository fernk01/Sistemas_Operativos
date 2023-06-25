/*  Program que funciona igual al sistem call: ls -al
 *  Como compilar:
 *      gcc -ggdb3 -O2 -Wall -Wextra -std=c11 -Wmissing-prototypes -Wvla
 * -D_DEFAULT_SOURCE ls.c -o ls
 *      ./ls
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <pwd.h>
#include <grp.h>

void traverse_directory(DIR *dirp);
void display_information(char *name);
void set_type_of_file(char *permissions, mode_t mode);
void set_permissions(char *permissions, mode_t mode);

int
main()
{
	DIR *directory = opendir(".");
	if (directory == NULL) {
		perror("Error en opendir");
		exit(-1);
	}

	traverse_directory(directory);

	closedir(directory);

	return 0;
}

/* Precondición: debe recibir un puntero al directorio que se desea explorar.
 * Postcondición: ninguna.
 * Efecto: explora el directorio y llama a la función display_information para
 * cada archivo encontrado.
 */
void
traverse_directory(DIR *dirp)
{
	struct dirent *entry;

	while ((entry = readdir(dirp)) != NULL) {
		display_information(entry->d_name);
	}
}
/* Precondición: debe recibir un puntero a una cadena con el nombre del archivo.
 * Postcondición: ninguna.
 * Efecto: obtiene y muestra información sobre el archivo, incluyendo permisos,
 * número de enlaces, propietario, grupo, tamaño, fecha y hora de modificación,
 * nombre del archivo y, en caso de ser un enlace simbólico, el archivo al que se refiere.
 */
void
display_information(char *name)
{
	struct stat sb;

	if (lstat(name, &sb) ==
	    -1) {  // lstat para verificar si un archivo es un enlace simbólico
		perror("Error: stat");
		exit(-1);
	}

	// Obtenemos una cadena de permisos que contiene el archivo.
	char permissions[11] = "----------";  // cantidad de permisos.
	set_type_of_file(permissions, sb.st_mode);
	set_permissions(permissions, sb.st_mode);

	// Obtenemos la hora de modificacion el archivo.
	char time_str[30];
	strftime(time_str, sizeof(time_str), "%b %d %H:%M", localtime(&sb.st_mtime));

	// Verificamos si es un symbolic_link
	char symbolic_link[1024];
	int len;
	if (permissions[0] == 'l') {
		len = readlink(name, symbolic_link, sizeof(symbolic_link) - 1);
		if (len != -1) {
			symbolic_link[len] = '\0';
		} else {
			perror("readlink");
			exit(EXIT_FAILURE);
		}
	}

	// nombre del propietario del archivo o directorio.
	struct passwd *pwd = getpwuid(sb.st_uid);
	// nombre del grupo al que pertenece el archivo o directorio.
	struct group *grp = getgrgid(sb.st_gid);

	// Mostramos pr pantalla.
	printf("%s %2ld %s %s %6lld  %s  %s %s %s\n",
	       permissions,         // Permisos, cadena de 10 caracteres.
	       (long) sb.st_nlink,  // Numero de enlace.
	       pwd->pw_name,  // Nombre del propietario del archivo o directorio.
	       grp->gr_name,  // Nombre del grupo al que pertenece el archivo o directorio.
	       (long long) sb.st_size,  // Tamaño del archivo en bytes.
	       time_str,                // Fecha y hora de Modificacion
	       name,                    // Nombre.
	       permissions[0] == 'l' ? "->" : "",
	       permissions[0] == 'l' ? symbolic_link : "");
}
/* Precondición: debe recibir un puntero a una cadena de permisos y un valor
 * mode_t que indica el modo del archivo. Postcondición: modifica la cadena de
 * permisos. Efecto: establece el carácter correspondiente al tipo de archivo
 * ('d' para directorio, '-' para archivo, 'l' para enlace simbólico).
 */
void
set_type_of_file(char *permissions, mode_t mode)
{
	if (S_ISDIR(mode))
		permissions[0] = 'd';
	else if (S_ISLNK(mode))
		permissions[0] = 'l';
	else if (S_ISREG(mode))
		permissions[0] = '-';
}
/* Precondición: debe recibir un puntero a una cadena de permisos y un valor
 * mode_t que indica el modo del archivo. Postcondición: modifica la cadena de
 * permisos. Efecto: establece los caracteres correspondientes a los permisos
 * del archivo ('r' para lectura, 'w' para escritura, 'x' para ejecución) en la
 * cadena de permisos.
 */
void
set_permissions(char *permissions, mode_t mode)
{
	if ((mode & S_IRUSR) == S_IRUSR)
		permissions[1] = 'r';
	if ((mode & S_IWUSR) == S_IWUSR)
		permissions[2] = 'w';
	if ((mode & S_IXUSR) == S_IXUSR)
		permissions[3] = 'x';
	if ((mode & S_IRGRP) == S_IRGRP)
		permissions[4] = 'r';
	if ((mode & S_IWGRP) == S_IWGRP)
		permissions[5] = 'w';
	if ((mode & S_IXGRP) == S_IXGRP)
		permissions[6] = 'x';
	if ((mode & S_IROTH) == S_IROTH)
		permissions[7] = 'r';
	if ((mode & S_IWOTH) == S_IWOTH)
		permissions[8] = 'w';
	if ((mode & S_IXOTH) == S_IXOTH)
		permissions[9] = 'x';
}
