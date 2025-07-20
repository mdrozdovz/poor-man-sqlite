#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>

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
	printf("\t-p\t\t\tPort to listen to for incoming connections\n");
	printf("\n");
}

int main(int argc, char *argv[]) {
	char *filepath = NULL;
	char *portarg = NULL;
	unsigned short port = 0;
	bool newfile = false;
	bool list = false;
	int c;

	while ((c = getopt(argc, argv, "nf:a:l")) != -1) {
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
				list = true;
				break;
			case 'h':
				print_usage(argv);
				return 0;
			case '?':
				printf("Unknown option -%c\n", c);
				break;
			default:
				print_usage(argv);
				return -1;
		}
	}

	if (filepath == NULL) {
		print_usage(argv);
		return 1;
	}

	int FD = open_db_file(filepath);
	if (FD == -1) {
		if (newfile) {
			FD = create_db_file(filepath);
		} else {
			printf("Failed to open db file\n");
			return 1;
		}
	}
}

