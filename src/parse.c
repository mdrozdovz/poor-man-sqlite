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

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {

}

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring) {

}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {

}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
	if (dbhdr == NULL) {
		printf("DB header is NULL\n");
		return STATUS_ERROR;
	}

	if (lseek(fd, 0, SEEK_SET) != 0) {
		printf("lseek failed\n");
		return STATUS_ERROR;
	}
	// dbhdr->magic = htonl(dbhdr->magic);
	// dbhdr->version = htons(dbhdr->version);
	// dbhdr->count = htons(dbhdr->count);
	// dbhdr->filesize = htonl(dbhdr->filesize);

	printf("Writing values, magic: %d version: %d count: %d filesize: %d\n", dbhdr->magic, dbhdr->version, dbhdr->count, dbhdr->filesize);

	if (write(fd, dbhdr, sizeof(struct dbheader_t)) == -1) {
		perror("write");
		return STATUS_ERROR;
	}

	return STATUS_SUCCESS;
}	

int validate_db_header(const int fd, struct dbheader_t **header_out) {
	if (header_out == NULL) {
		printf("DB header out pointer is NULL\n");
		return STATUS_ERROR;
	}

	if (fd < 0) {
		printf("Bad file descriptor provided\n");
		return STATUS_ERROR;
	}

	struct dbheader_t *dbhdr = calloc(1, sizeof(struct dbheader_t));
	lseek(fd, 0, SEEK_SET);
	const size_t res = read(fd, dbhdr, sizeof(struct dbheader_t));
	if (res != sizeof(struct dbheader_t)) {
		printf("Error reading db file: %lu vs %lu\n", res, sizeof(struct dbheader_t));
		free(dbhdr);
		return STATUS_ERROR;
	}
	// dbhdr->magic = ntohl(dbhdr->magic);
	// dbhdr->version = ntohs(dbhdr->version);
	// dbhdr->count = ntohs(dbhdr->count);
	// dbhdr->filesize = ntohl(dbhdr->magic);

	printf("Reading values, magic: %d, version: %d, count: %d, filesize: %d\n", dbhdr->magic, dbhdr->version, dbhdr->count, dbhdr->filesize);

	if (dbhdr->magic != HEADER_MAGIC) {
		printf("DB header magic is wrong\n");
		return STATUS_ERROR;
	}

	if (dbhdr->version != 1) {
		printf("Unsupported DB header version %d\n", dbhdr->version);
		return STATUS_ERROR;
	}

	struct stat *dbstat = malloc(sizeof(struct stat));
	const int rc = fstat(fd, dbstat);
	if (rc != 0) {
		printf("Error reading db file\n");
		free(dbstat);
		return STATUS_ERROR;
	}

	if (dbhdr->filesize != dbstat->st_size) {
		printf("DB header size is wrong, file is corrupted\n");
		free(dbstat);
		return STATUS_ERROR;
	}
	free(dbstat);

	*header_out = dbhdr;
	return STATUS_SUCCESS;
}

int create_db_header(struct dbheader_t **header_out) {
	if (header_out == NULL) {
		printf("DB header out pointer is NULL\n");
		return STATUS_ERROR;
	}

	struct dbheader_t *dbhdr = calloc(1, sizeof(struct dbheader_t));

	dbhdr->version = 1;
	dbhdr->count = 0;
	dbhdr->magic = HEADER_MAGIC;
	dbhdr->filesize = sizeof(struct dbheader_t);

	*header_out = dbhdr;
	return STATUS_SUCCESS;
}


