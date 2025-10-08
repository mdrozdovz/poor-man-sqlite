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

static int assert_non_null(void *p, char *path) {
	if (p == NULL) {
		printf("%s is required, but got NULL\n", path);
		return STATUS_ERROR;
	}

	return STATUS_SUCCESS;
}

void list_employees(dbheader_t *dbhdr, employee_t *employees) {

}

int add_employee(dbheader_t *dbhdr, employee_t **employees, char *addstring) {
	int assert_rc = STATUS_SUCCESS;
	assert_rc += assert_non_null(dbhdr, "dbhdr");
	assert_rc += assert_non_null(employees, "employees");
	assert_rc += assert_non_null(*employees, "*employees");
	assert_rc += assert_non_null(addstring, "addstring");

	if (assert_rc != STATUS_SUCCESS) {
		printf("add_employee() assert failed");
		return STATUS_ERROR;
	}

	employee_t *old_employees = *employees;
	employee_t *new_employees = realloc(*employees, (dbhdr->count + 1) * sizeof(employee_t));;
	if (new_employees == NULL) {
		printf("Failed to re-allocate memory for employees\n");
		free(old_employees);
		return STATUS_ERROR;
	}

	const char* name = strtok(addstring, ",");
	const char* address = strtok(NULL, ",");
	const char* hours = strtok(NULL, ",");

	printf("Adding employee: [%s] [%s] [%s] \n", name, address, hours);
	employee_t employee = *employees[dbhdr->count];

	strncpy(employee.name, name, sizeof(employee.name));
	strncpy(employee.address, address, sizeof(employee.address));
	employee.hours = atoi(hours);

	*employees = new_employees;
	dbhdr->count++;
	dbhdr->filesize = dbhdr->filesize + sizeof(employee);
	printf("Count on add: %d\n", dbhdr->count);
	return STATUS_SUCCESS;
}

int read_employees(int fd, dbheader_t *dbhdr, employee_t **employees_out) {
	const int count = dbhdr->count;

	employee_t *employees = calloc(count, sizeof(employee_t));
	if (employees == NULL) {
		printf("Failed to allocate memory for employees\n");
		return STATUS_ERROR;
	}

	read(fd, employees, count);
	for (int i = 0; i < count; i++) {
		employees[i].hours = ntohl(employees[i].hours);
	}

	*employees_out = employees;
	return STATUS_SUCCESS;
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
	printf("Original values, magic: %d, version: %d, count: %d, filesize: %d\n", dbhdr->magic, dbhdr->version, dbhdr->count, dbhdr->filesize);

	dbhdr->magic = htonl(dbhdr->magic);
	dbhdr->version = htons(dbhdr->version);
	dbhdr->count = htons(dbhdr->count);
	dbhdr->filesize = htonl(dbhdr->filesize);

	printf("Writing values, magic: %d version: %d count: %d filesize: %d\n", dbhdr->magic, dbhdr->version, dbhdr->count, dbhdr->filesize);

	if (write(fd, dbhdr, sizeof(dbheader_t)) == -1) {
		perror("write");
		return STATUS_ERROR;
	}

	if (employees == NULL) {
		printf("Employees is NULL, skipping writing\n");
	} else {
		printf("Count on write: %d\n", ntohs(dbhdr->count));
		if (write(fd, employees, sizeof(employee_t) * ntohs(dbhdr->count)) == -1) {
			perror("write");
			return STATUS_ERROR;
		}
	}

	return STATUS_SUCCESS;
}	

int validate_db_header(const int fd, dbheader_t **header_out) {
	if (header_out == NULL) {
		printf("DB header out pointer is NULL\n");
		return STATUS_ERROR;
	}

	if (fd < 0) {
		printf("Bad file descriptor provided\n");
		return STATUS_ERROR;
	}

	dbheader_t *dbhdr = calloc(1, sizeof(dbheader_t));
	lseek(fd, 0, SEEK_SET);
	const size_t res = read(fd, dbhdr, sizeof(dbheader_t));
	if (res != sizeof(dbheader_t)) {
		printf("Error reading db file: %lu vs %lu\n", res, sizeof(dbheader_t));
		free(dbhdr);
		return STATUS_ERROR;
	}
	printf("Reading values, magic: %d, version: %d, count: %d, filesize: %d\n", dbhdr->magic, dbhdr->version, dbhdr->count, dbhdr->filesize);

	dbhdr->magic = ntohl(dbhdr->magic);
	dbhdr->version = ntohs(dbhdr->version);
	dbhdr->count = ntohs(dbhdr->count);
	dbhdr->filesize = ntohl(dbhdr->filesize);

	printf("Converted values, magic: %d, version: %d, count: %d, filesize: %d\n", dbhdr->magic, dbhdr->version, dbhdr->count, dbhdr->filesize);

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

int create_db_header(dbheader_t **header_out) {
	if (header_out == NULL) {
		printf("DB header out pointer is NULL\n");
		return STATUS_ERROR;
	}

	dbheader_t *dbhdr = calloc(1, sizeof(dbheader_t));

	dbhdr->version = 1;
	dbhdr->count = 0;
	dbhdr->magic = HEADER_MAGIC;
	dbhdr->filesize = sizeof(dbheader_t);

	*header_out = dbhdr;
	return STATUS_SUCCESS;
}


