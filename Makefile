# declare the 'clean' target to be a phony rule
.PHONY: clean

# define compiler options
CFLAGS=-Wall -Wextra
CC=gcc

# rule to build the executable program from object files
wrdsh: wrdsh.o stack.o

# rules to build object files
wrdsh.o: wrdsh.c stack.h

stack.o: stack.c stack.h

# rules to remove all build targets
clean:
	rm -f *.o wrdsh
