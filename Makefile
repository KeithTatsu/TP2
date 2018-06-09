CC = gcc
CFLAGS = -g -std=c99 -Wall -Wconversion -Wno-sign-conversion -Wtype-limits -pedantic
EXEC = pruebas

compile: tp2.c heap.c heap.h strutil.c strutil.h lista.c lista.h hash.c hash.h
	$(CC) $(CFLAGS) tp2.c heap.c strutil.c lista.c hash.c -o $(EXEC)

run: compile
	./$(EXEC)

valgrind: compile
	valgrind --track-origins=yes --leak-check=full ./$(EXEC) 

gdb: compile
	gdb ./$(EXEC) -tui

gui: compile
	gdbgui ./$(EXEC)
