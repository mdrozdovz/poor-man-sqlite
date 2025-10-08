#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include "common.h"
#include "parse.h"

#include <stdbool.h>

static int assert_non_null(void *p, char *path) {
    if (p == NULL) {
        printf("%s is required, but got NULL\n", path);
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}

static void print_employees(dbheader_t *dbhdr, employee_t *employees) {
    for (int i = 0; i < dbhdr->count; i++) {
        employee_t emp = employees[i];
        printf("Employee name: %s, address: %s, hours: %d\n", emp.name, emp.address, emp.hours);
    }
}

void debug(const char *format, ...) {
    if (!PARSE_DEBUG_PRINT) return;

    va_list vlist;
    va_start(vlist, format);

    vfprintf(stdout, format, vlist);

    va_end(vlist);
}

void list_employees(dbheader_t *dbhdr, employee_t *employees) {
    print_employees(dbhdr, employees);
}

int add_employee(dbheader_t *dbhdr, employee_t **employees_inout, char *addstring) {
    int assert_rc = STATUS_SUCCESS;
    assert_rc += assert_non_null(dbhdr, "dbhdr");
    assert_rc += assert_non_null(employees_inout, "employees");
    assert_rc += assert_non_null(*employees_inout, "*employees");
    assert_rc += assert_non_null(addstring, "addstring");

    if (assert_rc != STATUS_SUCCESS) {
        printf("add_employee() assert failed");
        return STATUS_ERROR;
    }

    employee_t *old_employees = *employees_inout;
    employee_t *new_employees = realloc(*employees_inout, (dbhdr->count + 1) * sizeof(employee_t));
    debug("Old p: %p, new p: %p\n", *employees_inout, new_employees);
    if (new_employees == NULL) {
        printf("Failed to re-allocate memory for employees\n");
        free(old_employees);
        return STATUS_ERROR;
    }

    const char *name = strtok(addstring, ",");
    const char *address = strtok(NULL, ",");
    const char *hours = strtok(NULL, ",");

    debug("Before add\n");
    if (PARSE_DEBUG_PRINT)
        print_employees(dbhdr, new_employees);
    debug("Adding employee: [%s] [%s] [%s] \n", name, address, hours);
    employee_t *employee = &new_employees[dbhdr->count];

    strncpy(employee->name, name, sizeof(employee->name));
    strncpy(employee->address, address, sizeof(employee->address));
    employee->hours = atoi(hours);

    *employees_inout = new_employees;
    dbhdr->count++;
    dbhdr->filesize = dbhdr->filesize + sizeof(*employee);
    debug("Count on add: %d\n", dbhdr->count);
    return STATUS_SUCCESS;
}

int read_employees(int fd, dbheader_t *dbhdr, employee_t **employees_out) {
    const int count = dbhdr->count;

    employee_t *employees = calloc(count, sizeof(employee_t));
    if (employees == NULL) {
        printf("Failed to allocate memory for employees\n");
        return STATUS_ERROR;
    }

    read(fd, employees, count * sizeof(employee_t));
    for (int i = 0; i < count; i++) {
        employees[i].hours = ntohl(employees[i].hours);
    }
    if (PARSE_DEBUG_PRINT)
        print_employees(dbhdr, employees);

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
    debug("Original values, magic: %d, version: %d, count: %d, filesize: %d\n", dbhdr->magic, dbhdr->version,
           dbhdr->count, dbhdr->filesize);
    if (PARSE_DEBUG_PRINT)
        print_employees(dbhdr, employees);

    dbhdr->magic = htonl(dbhdr->magic);
    dbhdr->version = htons(dbhdr->version);
    dbhdr->count = htons(dbhdr->count);
    dbhdr->filesize = htonl(dbhdr->filesize);

    debug("Writing values, magic: %d version: %d count: %d filesize: %d\n", dbhdr->magic, dbhdr->version, dbhdr->count,
           dbhdr->filesize);

    size_t bytes_written = write(fd, dbhdr, sizeof(dbheader_t));
    if (bytes_written == -1) {
        perror("write");
        return STATUS_ERROR;
    }
    debug("Written %lu bytes of header\n", bytes_written);

    if (employees == NULL) {
        printf("Employees is NULL, skipping writing\n");
    } else {
        debug("Count on write: %d\n", ntohs(dbhdr->count));
        for (int i = 0; i < ntohs(dbhdr->count); i++) {
            employees[i].hours = htonl(employees[i].hours);
        }
        bytes_written = write(fd, employees, sizeof(employee_t) * ntohs(dbhdr->count));

        if (bytes_written == -1) {
            perror("write");
            return STATUS_ERROR;
        }
        debug("Written %lu bytes of employees\n", bytes_written);
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
    debug("Reading values, magic: %d, version: %d, count: %d, filesize: %d\n", dbhdr->magic, dbhdr->version,
           dbhdr->count, dbhdr->filesize);

    dbhdr->magic = ntohl(dbhdr->magic);
    dbhdr->version = ntohs(dbhdr->version);
    dbhdr->count = ntohs(dbhdr->count);
    dbhdr->filesize = ntohl(dbhdr->filesize);

    debug("Converted values, magic: %d, version: %d, count: %d, filesize: %d\n", dbhdr->magic, dbhdr->version,
           dbhdr->count, dbhdr->filesize);

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
