
# File: Makefile


all: json.c json.h ppmrw_io.c ppmrw_io.h raycast.c
	gcc raycast.c json.c ppmrw_io.c -o raycast

json.o: json.c json.h
	gcc -c json.c -o json.o
	
ppmrw_io.o: ppmrw_io.c ppmrw_io.h
	gcc -c ppmrw_io.c -o ppmrw_io.o

clean:
	rm *.o raycast
