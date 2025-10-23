#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
    printf("Usage: %s [options]\n", argv[0]);
    printf("Options:\n");
    printf("\t-h\t\t\tShow this help and exit\n");
    printf("\t-n\t\t\tCreate new file if it does not exist\n");
    printf("\t-f <filepath>\t\tDB file path\n");
    printf("\t-l\t\t\tList all employees\n");
    printf("\t-u\t\t\tUpdate employee's work hours by a given name\n");
    printf("\t-d\t\t\tDelete employee by name\n");
    printf("\t-p\t\t\tPort to listen to for incoming connections\n");
    printf("\n");
}

int main(int argc, char *argv[]) {
    char *filepath = NULL;
    char *portarg = NULL;
    char *add_record = NULL;
    char *delete_name = NULL;
    char *update_record = NULL;
    unsigned short port = 0;
    bool newfile = false;
    bool list_records = false;
    int c;

    dbheader_t *dbhdr = NULL;
    employee_t *employees = NULL;

    while ((c = getopt(argc, argv, "nlf:a:u:d:")) != -1) {
        switch (c) {
            case 'n':
                newfile = true;
                break;
            case 'f':
                filepath = optarg;
                break;
            case 'p':
                portarg = optarg;
                break;
            case 'l':
                list_records = true;
                break;
            case 'a':
                add_record = optarg;
                break;
            case 'd':
                delete_name = optarg;
                break;
            case 'u':
                update_record = optarg;
                break;
            case 'h':
                print_usage(argv);
                return 0;
            default:
                print_usage(argv);
                return -1;
        }
    }

    if (filepath == NULL) {
        print_usage(argv);
        return 1;
    }

    int fd;
    int rc = 0;
    if (file_exists(filepath)) {
        fd = open_db_file(filepath);
    } else if (newfile) {
        fd = create_db_file(filepath);
        rc = create_db_header(&dbhdr);
        if (rc != STATUS_SUCCESS) {
            printf("Failed to create db header\n");
            return rc;
        }
        rc = output_file(fd, dbhdr, NULL);
        if (rc != STATUS_SUCCESS) {
            printf("Failed to write to db file\n");
            return rc;
        }
    } else {
        printf("Failed to open db file\n");
        return 1;
    }

    if (fd == -1) {
        printf("Failed to open db file\n");
        return 1;
    }

    if (validate_db_header(fd, &dbhdr) != STATUS_SUCCESS) {
        printf("DB header is invalid\n");
        close(fd);
        return 1;
    }

    rc = read_employees(fd, dbhdr, &employees);
    printf("employees after read: %p\n", employees);
    if (rc != STATUS_SUCCESS) {
        printf("Failed to read employees\n");
        return rc;
    }

    if (add_record != NULL) {
        rc = add_employee(dbhdr, &employees, add_record);
        if (rc != STATUS_SUCCESS) {
            printf("Failed to add employee\n");
            close(fd);
            return rc;
        }
    }

    if (update_record != NULL) {
        rc = update_employee_hours(dbhdr, employees, update_record);
        if (rc != STATUS_SUCCESS) {
            printf("Failed to update employee's work hours\n");
            close(fd);
            return rc;
        }
    }

    if (delete_name != NULL) {
        rc = delete_employee(dbhdr, &employees, delete_name);
        if (rc != STATUS_SUCCESS) {
            printf("Failed to delete employee\n");
            close(fd);
            return rc;
        }
    }

    if (list_records) {
        list_employees(dbhdr, employees);
    }

    output_file(fd, dbhdr, employees);
    free(dbhdr);
    free(employees);
    close(fd);

    return 0;
}
