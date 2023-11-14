#--------------------------------------------------------------------
# Makefile for Assignment 4, Part 3 sample implementation
# Invoke with the command:
# 	make -f Makefile.sampleft
# Author: Siling Song & Alejandra Duran
#--------------------------------------------------------------------


all: testFT

clean:
	rm -f testFT meminfo*.out

clobber: clean
	rm -f ft_client.o dynarray.o path.o nodeFT.o ft.o *~

testFT: ft_client.o dynarray.o path.o nodeFT.o ft.o
	gcc217 ft_client.o dynarray.o path.o nodeFT.o ft.o -o testFT

ft_client.o: ft_client.c ft.h a4def.h
	gcc217 -c ft_client.c

dynarray.o: dynarray.c dynarray.h
	gcc217 -c dynarray.c	

path.o: path.c dynarray.h path.h a4def.h
	gcc217 -c path.c

nodeFT.o: nodeFT.c dynarray.h path.h ft.h a4def.h
	gcc217 -c nodeFT.c	

ft.o: ft.c dynarray.h path.h ft.h a4def.h	
	gcc217 -c ft.c