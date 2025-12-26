SRV_TARGET = bin/dbview
CLI_TARGET = bin/client
SRV_SRC = $(wildcard src/*.c)
CLI_SRC = $(wildcard client/*.c)
SRV_OBJ = $(patsubst src/%.c, build/%.o, $(SRV_SRC))
CLI_OBJ = $(patsubst src/%.c, build/%.o, $(CLI_SRC))

run: clean default
	./$(SRV_TARGET) -f ./mynewdb.db -n
	@echo "===== Adding ====="
	./$(SRV_TARGET) -f ./mynewdb.db -a "Timmy H.,123 Sheshire Ln.,120"
	./$(SRV_TARGET) -f ./mynewdb.db -a "Tony A.,456 Sheshire Ln.,40"
	./$(SRV_TARGET) -f ./mynewdb.db -l
	@echo "===== Updating ====="
	./$(SRV_TARGET) -f ./mynewdb.db -u "Tony A.,60"
	./$(SRV_TARGET) -f ./mynewdb.db -l
	@echo "===== Deleting ====="
	./$(SRV_TARGET) -f ./mynewdb.db -d "Timmy H."
	./$(SRV_TARGET) -f ./mynewdb.db -l


run-server: clean default
	./$(SRV_TARGET) -f ./mynewdb.db -n -p 3412

default: SRV_TARGET CLI_TARGET

clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f *.db

$(SRV_TARGET): SRV_OBJ
	gcc -o $@ $?

$(CLI_TARGET): CLI_OBJ
	gcc -o $@ $?

build/%.o: src/%.c
	gcc -c $< -o $@ -Iinclude


