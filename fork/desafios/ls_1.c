
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include <string.h>

void print_permissions(mode_t mode) {
    printf((S_ISDIR(mode)) ? "d" : "-");
    printf((mode & S_IRUSR) ? "r" : "-");
    printf((mode & S_IWUSR) ? "w" : "-");
    printf((mode & S_IXUSR) ? "x" : "-");
    printf((mode & S_IRGRP) ? "r" : "-");
    printf((mode & S_IWGRP) ? "w" : "-");
    printf((mode & S_IXGRP) ? "x" : "-");
    printf((mode & S_IROTH) ? "r" : "-");
    printf((mode & S_IWOTH) ? "w" : "-");
    printf((mode & S_IXOTH) ? "x" : "-");
}

void print_entry(const char *path, const struct dirent *entry) {
    struct stat entry_stat;
    if (lstat(path, &entry_stat) == -1) {
        perror("lstat");
        return;
    }

    printf("%s ", entry->d_name);
    print_permissions(entry_stat.st_mode);
    printf(" %d ", entry_stat.st_uid);

    if (S_ISLNK(entry_stat.st_mode)) {
        char link_target[1024];
        ssize_t link_length = readlink(path, link_target, sizeof(link_target) - 1);
        if (link_length == -1) {
            perror("readlink");
            return;
        }
        link_target[link_length] = '\0';
        printf("-> %s", link_target);
    }

    printf("\n");
}

int main(int argc, char *argv[]) {
    const char *directory_path;
    if (argc == 2) {
        directory_path = argv[1];
    } else {
        directory_path = ".";
    }

    DIR *directory = opendir(directory_path);
    if (directory == NULL) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", directory_path, entry->d_name);
        print_entry(path, entry);
    }

    closedir(directory);
    return 0;
}
