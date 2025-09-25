#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "parse.h"

void list_employees(dbheader_t *dbhdr, employee_t *employees) {

}

int add_employee(dbheader_t *dbhdr, employee_t *employees, char *addstring) {

}

int read_employees(int fd, dbheader_t *dbhdr, employee_t **employeesOut) {

}

int output_file(int fd, dbheader_t *dbhdr, employee_t *employees) {
	if (dbhdr == NULL) {
		printf("DB header is NULL\n");
		return STATUS_ERROR;
	}

	if (lseek(fd, 0, SEEK_SET) != 0) {
		printf("lseek failed\n");
		return STATUS_ERROR;
	}

	if (write(fd, dbhdr, sizeof(dbheader_t)) == -1) {
		perror("write");
		return STATUS_ERROR;
	};

	return STATUS_SUCCESS;
}	

int validate_db_header(int fd, dbheader_t **header_out) {
	if (fd < 0) {
		printf("Bad file descriptor provided\n");
		return STATUS_ERROR;
	}

	dbheader_t *dbhdr = calloc(1, sizeof(dbheader_t));
	const int res = read(fd, dbhdr, sizeof(dbheader_t));
	if (res != sizeof(dbheader_t)) {
		printf("Error reading db file\n");
		return STATUS_ERROR;
	}

	*header_out = dbhdr;
	return STATUS_SUCCESS;
}

int create_db_header(dbheader_t **header_out) {
	dbheader_t *dbhdr = calloc(1, sizeof(dbheader_t));

	dbhdr->version = 1;
	dbhdr->count = 0;
	dbhdr->magic = HEADER_MAGIC;
	dbhdr->filesize = sizeof(dbheader_t);

	*header_out = dbhdr;
	return STATUS_SUCCESS;
}


