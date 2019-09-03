all: dct quality 

dct: dct.c
	gcc -Wall --static ${CFLAGS} -I../../../../src/include/ -O3  dct.c -c
	gcc -Wall -O3  -Wall --static ${CFLAGS} -I../../../../src/include/ -Wall -O3 ../../../../src/lib/m5op_x86.o ../../../../src/lib/m5_mmap.o dct.c -o  dct -lm
quality: quality.c
	gcc -Wall -O3   quality.c -o  quality -lm

clean:
	rm -f dct quality *.o *.out *.raw 

