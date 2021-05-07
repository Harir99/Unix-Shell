CC=gcc 
CFLAGS=-Wpedantic -std=gnu99 -Wall -g 

all: myShell

myShell: myShell.c myShell.h
	$(CC) $(CFLAGS) myShell.c -o myShell

myShell.o: myShell.c myShell.h
	$(CC) $(CFLAGS) -c myShell.c -o myShell.o

clean:
	rm -f *.o myShell
