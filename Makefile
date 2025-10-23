TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))

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


default: $(TARGET)

clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f *.db

$(TARGET): $(OBJ)
	gcc -o $@ $?

build/%.o : src/%.c
	gcc -c $< -o $@ -Iinclude


