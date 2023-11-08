CC = gcc
CFLAGS = -g -W -Wall

all:
	make part1
	make part2
	make part3
	make part4

part1: part1.o MCP.o
	$(CC) $(CFLAGS) -o part1 part1.o MCP.o

part1.o: part1.c MCP.h
	$(CC) $(CFLAGS) -c part1.c

part2: part2.o MCP.o
	$(CC) $(CFLAGS) -o part2 part2.o MCP.o

part2.o: part2.c MCP.h
	$(CC) $(CFLAGS) -c part2.c

part3: part3.o MCP.o
	$(CC) $(CFLAGS) -o part3 part3.o MCP.o

part3.o: part3.c MCP.h
	$(CC) $(CFLAGS) -c part3.c

part4: part4.o MCP.o
	$(CC) $(CFLAGS) -o part4 part4.o MCP.o

part4.o: part4.c MCP.h
	$(CC) $(CFLAGS) -c part4.c

MCP.o: MCP.c MCP.h
	$(CC) $(CFLAGS) -c MCP.c

clean:
	rm -f *.o part1 part2 part3 part4

