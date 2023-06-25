#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

int
main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("Uso: %s src dst\n", argv[0]);
		return 1;
	}

	const char *src_path = argv[1];
	const char *dst_path = argv[2];

	int src_fd = open(src_path, O_RDONLY);
	if (src_fd == -1) {
		perror("Error al abrir el archivo fuente");
		return 1;
	}

	struct stat src_stat;
	if (fstat(src_fd, &src_stat) == -1) {
		perror("Error al obtener información del archivo fuente");
		close(src_fd);
		return 1;
	}

	if (!S_ISREG(src_stat.st_mode)) {
		printf("El archivo fuente no es regular\n");
		close(src_fd);
		return 1;
	}

	int dst_fd =
	        open(dst_path, O_WRONLY | O_CREAT | O_EXCL, src_stat.st_mode);
	if (dst_fd == -1) {
		perror("Error al crear el archivo destino");
		close(src_fd);
		return 1;
	}

	off_t file_size = src_stat.st_size;
	if (ftruncate(dst_fd, file_size) == -1) {
		perror("Error al redimensionar el archivo destino");
		close(src_fd);
		close(dst_fd);
		return 1;
	}

	void *src_addr = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
	if (src_addr == MAP_FAILED) {
		perror("Error al mapear el archivo fuente");
		close(src_fd);
		close(dst_fd);
		return 1;
	}

	void *dst_addr = mmap(NULL, file_size, PROT_WRITE, MAP_SHARED, dst_fd, 0);
	if (dst_addr == MAP_FAILED) {
		perror("Error al mapear el archivo destino");
		munmap(src_addr, file_size);
		close(src_fd);
		close(dst_fd);
		return 1;
	}

	memcpy(dst_addr, src_addr, file_size);

	if (munmap(src_addr, file_size) == -1) {
		perror("Error al desmapear el archivo fuente");
		close(src_fd);
		close(dst_fd);
		return 1;
	}

	if (munmap(dst_addr, file_size) == -1) {
		perror("Error al desmapear el archivo destino");
		close(src_fd);
		close(dst_fd);
		return 1;
	}

	close(src_fd);
	close(dst_fd);

	return 0;
}
