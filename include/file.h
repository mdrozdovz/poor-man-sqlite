#ifndef FILE_H
#define FILE_H

#include <stdbool.h>

int create_db_file(const char *filename);
int open_db_file(const char *filename);
bool file_exists(char *filename);

#endif
