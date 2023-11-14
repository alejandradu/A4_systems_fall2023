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
	rm -f ft_client.o dynarray.o path.o nodeFT.o FT.o *~

testFT: ft_client.o dynarray.o path.o nodeFT.o FT.o
	gcc217 ft_client.o dynarray.o path.o nodeFT.o FT.o -o testFT

ft_client.o: ft_client.c FT.h a4def.h
	gcc217 -c ft_client.c

dynarray.o: dynarray.c dynarray.h
	gcc217 -c dynarray.c	

path.o: path.c dynarray.h path.h a4def.h
	gcc217 -c path.c

nodeFT.o: nodeFT.c dynarray.h path.h FT.h a4def.h
	gcc217 -c nodeFT.c	

FT.o: FT.c dynarray.h path.h FT.h a4def.h	
	gcc217 -c FT.c