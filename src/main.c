#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>

#include "../include/server.h"
#include "../include/file.h"
#include "../include/common.h"
#include "../include/parse.h"

void print_usage(char *argv[]) {
    printf("Usage: %s [options]\n", argv[0]);
    printf("Options:\n");
    printf("\t-h\t\t\tShow this help and exit\n");
    printf("\t-n\t\t\tCreate new file if it does not exist\n");
    printf("\t-f <filepath>\t\tDB file path\n");
    printf("\t-l\t\t\tList all employees\n");
    printf("\t-a <name,address,hours>\tAdd employee record\n");
    printf("\t-u <name,hours>\t\tUpdate employee's work hours by a given name\n");
    printf("\t-d <name>\t\tDelete employee by name\n");
    printf("\t-p <port>\t\tPort to listen to for incoming connections\n");
    printf("\n");
}

typedef struct cli_options {
    char *filepath;
    char *add_record;
    char *delete_name;
    char *update_record;
    unsigned short port;
    bool newfile;
    bool list_records;
    bool print_usage;
} cli_options;

cli_options parse_args(int argc, char *argv[]) {
    cli_options opts = {0};
    int c;
    while ((c = getopt(argc, argv, "nlf:a:u:d:p:")) != -1) {
        switch (c) {
            case 'n':
                opts.newfile = true;
                break;
            case 'f':
                opts.filepath = optarg;
                break;
            case 'p':
                opts.port = (unsigned short) atoi(optarg);
                break;
            case 'l':
                opts.list_records = true;
                break;
            case 'a':
                opts.add_record = optarg;
                break;
            case 'd':
                opts.delete_name = optarg;
                break;
            case 'u':
                opts.update_record = optarg;
                break;
            case 'h':
            default:
                opts.print_usage = true;
                break;
        }
    }

    return opts;
}

typedef struct open_resources {
    int dbfd;
    dbheader_t *dbhdr;
    employee_t *employees;
} open_resources_t;

static open_resources_t open_res = {0};

static void cleanup(const int sig) {
    if (sig >= 0) printf("\nGot signal %d\n ", sig);
    printf("Saving data and cleaning up\n");

    stop_server();
    if (open_res.dbfd > 0 && open_res.dbhdr != NULL && open_res.employees != NULL) {
        output_file(open_res.dbfd, open_res.dbhdr, open_res.employees);
        free(open_res.dbhdr);
        free(open_res.employees);
    }
    if (open_res.dbfd > 0)
        close(open_res.dbfd);

    exit(0);
}

static void continue_execution(const int sig) {
    if (sig >= 0) printf("\nGot signal %d\n ", sig);
}

int main(int argc, char *argv[]) {
    struct sigaction act = {0};
    act.sa_flags = SA_RESETHAND;
    act.sa_sigaction = (void (*)(int, siginfo_t *, void *)) &cleanup;
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGPIPE, &act, NULL);

    dbheader_t *dbhdr = NULL;
    employee_t *employees = NULL;

    const cli_options opts = parse_args(argc, argv);
    if (opts.print_usage) {
        print_usage(argv);
        return 0;
    }

    if (opts.filepath == NULL) {
        print_usage(argv);
        return 1;
    }

    int fd;
    int rc = 0;
    if (file_exists(opts.filepath)) {
        fd = open_db_file(opts.filepath);
    } else if (opts.newfile) {
        fd = create_db_file(opts.filepath);
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
    open_res.dbfd = fd;


    if (validate_db_header(fd, &dbhdr) != STATUS_SUCCESS) {
        printf("DB header is invalid\n");
        close(fd);
        return 1;
    }
    open_res.dbhdr = dbhdr;

    rc = read_employees(fd, dbhdr, &employees);
    printf("employees after read: %p\n", employees);
    if (rc != STATUS_SUCCESS) {
        printf("Failed to read employees\n");
        return rc;
    }
    open_res.employees = employees;

    if (opts.add_record != NULL) {
        rc = add_employee(dbhdr, &employees, opts.add_record);
        if (rc != STATUS_SUCCESS) {
            printf("Failed to add employee\n");
            close(fd);
            return rc;
        }
    }

    if (opts.update_record != NULL) {
        rc = update_employee_hours(dbhdr, employees, opts.update_record);
        if (rc != STATUS_SUCCESS) {
            printf("Failed to update employee's work hours\n");
            close(fd);
            return rc;
        }
    }

    if (opts.delete_name != NULL) {
        rc = delete_employee(dbhdr, &employees, opts.delete_name);
        if (rc != STATUS_SUCCESS) {
            printf("Failed to delete employee\n");
            close(fd);
            return rc;
        }
    }

    if (opts.list_records) {
        list_employees(dbhdr, employees);
    }

    if (opts.port != 0) {
        const int sfd = start_server(opts.port, 0);
        if (sfd == -1) {
            cleanup(-1);
            return 1;
        }

        while (true) {
            tick_server();
        }
    }

    cleanup(-1);
    return 0;
}
