#--------------------------------------------------------------------
# Makefile for Assignment 4, Part 3 sample implementation
# Invoke with the command:
# 	make -f Makefile.sampleft
# Author: Siling Song & Alejandra Duran
#--------------------------------------------------------------------

CC = gcc217
CFLAGS = -Wall -Wextra -pedantic -std=c99
LDFLAGS =

all: testFT

clean:
	rm -f testFT meminfo*.out *.o

testFT: ft_client.o dynarray.o path.o nodeFT.o ft.o
	$(CC) $(CFLAGS) -g $^ -o $@ $(LDFLAGS)

ft_client.o: ft_client.c ft.h a4def.h
	$(CC) $(CFLAGS) -g -c $< -o $@

dynarray.o: dynarray.c dynarray.h
	$(CC) $(CFLAGS) -g -c $< -o $@

path.o: path.c dynarray.h path.h a4def.h
	$(CC) $(CFLAGS) -g -c $< -o $@

nodeFT.o: nodeFT.c dynarray.h path.h ft.h a4def.h
	$(CC) $(CFLAGS) -g -c $< -o $@

ft.o: ft.c dynarray.h path.h ft.h a4def.h	
	$(CC) $(CFLAGS) -g -c $< -o $@