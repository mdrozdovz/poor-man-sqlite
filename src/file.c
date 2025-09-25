#include <stdio.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "file.h"
#include "common.h"


int create_db_file(const char *filename) {
    int fd = open(filename,O_RDWR);
    if (fd != -1) {
        printf("File %s already exists\n", filename);
        return STATUS_ERROR;
    }

    fd = open(filename, O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
        perror("open create");
        return STATUS_ERROR;
    }

    return fd;
}

int open_db_file(const char *filename) {
    const int fd = open(filename,O_RDWR);
    if (fd == -1) {
        perror("open");
        return STATUS_ERROR;
    }

    return fd;
}

bool file_exists(char *filename) {
    const int fd = open(filename,O_RDWR);
    return fd != -1;
}
