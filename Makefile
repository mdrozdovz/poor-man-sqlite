OBJ_DIR = obj
BIN_DIR = bin
DEP_DIR = include
DATA_DIR = data

SRV_TARGET = $(BIN_DIR)/dbview
CLI_TARGET = $(BIN_DIR)/client

SRV_SRC = $(wildcard src/*.c)
CLI_SRC = $(wildcard client/*.c)
SRV_OBJ = $(SRV_SRC:src/%.c=$(OBJ_DIR)/%.o)
CLI_OBJ = $(CLI_SRC:client/%.c=$(OBJ_DIR)/%.o)

DB_FILE = ./data/employees.dat

WARNINGS = -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align \
            -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations \
            -Wredundant-decls -Wnested-externs -Winline -Wno-long-long \
            -Wconversion -Wstrict-prototypes
CFLAGS = -g -I$(DEP_DIR) $(WARNINGS)


run: clean build
	./$(SRV_TARGET) -f $(DB_FILE) -n
	@echo "===== Adding ====="
	./$(SRV_TARGET) -f $(DB_FILE) -a "Timmy H.,123 Sheshire Ln.,120"
	./$(SRV_TARGET) -f $(DB_FILE) -a "Tony A.,456 Sheshire Ln.,40"
	./$(SRV_TARGET) -f $(DB_FILE) -l
	@echo "===== Updating ====="
	./$(SRV_TARGET) -f $(DB_FILE) -u "Tony A.,60"
	./$(SRV_TARGET) -f $(DB_FILE) -l
	@echo "===== Deleting ====="
	./$(SRV_TARGET) -f $(DB_FILE) -d "Timmy H."
	./$(SRV_TARGET) -f $(DB_FILE) -l


run-server: clean build
	./$(SRV_TARGET) -f $(DB_FILE) -n -p 3412

build: $(SRV_TARGET) $(CLI_TARGET)

clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(BIN_DIR)/*
	rm -f $(DATA_DIR)/*.db

$(SRV_TARGET): $(SRV_OBJ)
	gcc $(CFLAGS) -o $@ $?

$(SRV_OBJ): obj/%.o: src/%.c
	gcc $(CFLAGS) -c -o $@ $?

$(CLI_TARGET): $(CLI_OBJ)
	gcc $(CFLAGS) -o $@ $?

$(CLI_OBJ): obj/%.o: client/%.c
	gcc $(CFLAGS) -c -o $@ $?
