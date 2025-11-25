TARGET = bin/dbview
CLIENT_TARGET = bin/client
SRC = $(wildcard src/*.c)
CLIENT_SRC = $(wildcard client/*.c)
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))
CLIENT_OBJ = $(patsubst src/%.c, build/%.o, $(CLIENT_SRC))

run: clean default
	./$(TARGET) -f ./mynewdb.db -n
	@echo "===== Adding ====="
	./$(TARGET) -f ./mynewdb.db -a "Timmy H.,123 Sheshire Ln.,120"
	./$(TARGET) -f ./mynewdb.db -a "Tony A.,456 Sheshire Ln.,40"
	./$(TARGET) -f ./mynewdb.db -l
	@echo "===== Updating ====="
	./$(TARGET) -f ./mynewdb.db -u "Tony A.,60"
	./$(TARGET) -f ./mynewdb.db -l
	@echo "===== Deleting ====="
	./$(TARGET) -f ./mynewdb.db -d "Timmy H."
	./$(TARGET) -f ./mynewdb.db -l


run-server: clean default
	./$(TARGET) -f ./mynewdb.db -n -p 3412

default: $(TARGET) $(CLIENT_TARGET)

clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f *.db

$(TARGET): $(OBJ)
	gcc -o $@ $?

$(CLIENT_TARGET): $(CLIENT_OBJ)
	gcc -o $@ $?

build/%.o: src/%.c
	gcc -c $< -o $@ -Iinclude


