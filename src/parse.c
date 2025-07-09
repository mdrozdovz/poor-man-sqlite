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

}	

int validate_db_header(int fd, dbheader_t **headerOut) {

}

int create_db_header(int fd, dbheader_t **headerOut) {
	
}


