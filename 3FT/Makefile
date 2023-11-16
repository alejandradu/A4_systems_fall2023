#--------------------------------------------------------------------
# Makefile for Assignment 4, Part 3 sample implementation
# Invoke with the command:
# 	make -f Makefile.sampleft
# Author: Siling Song & Alejandra Duran
#--------------------------------------------------------------------

CC = gcc217

all: testFT

clean:
	rm -f testFT meminfo*.out *.o

testFT: ft_client.o dynarray.o path.o nodeFT.o ft.o
	$(CC) -g ft_client.o dynarray.o path.o nodeFT.o ft.o -o testFT

ft_client.o: ft_client.c ft.h a4def.h
	$(CC) -g -c ft_client.c -o ft_client.o

dynarray.o: dynarray.c dynarray.h
	$(CC) -g -c dynarray.c -o dynarray.o

path.o: path.c dynarray.h path.h a4def.h
	$(CC) -g -c path.c -o path.o

nodeFT.o: nodeFT.c dynarray.h path.h ft.h a4def.h
	$(CC) -g -c nodeFT.c -o nodeFT.o

ft.o: ft.c dynarray.h path.h ft.h a4def.h	
	$(CC) -g -c ft.c -o ft.o