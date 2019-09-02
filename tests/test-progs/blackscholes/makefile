all: blackscholes error

blackscholes: blackscholes.c
	gcc ${CFLAGS} -I../../../../src/include/ -Wall -O3 -static   blackscholes.c -c
	gcc ${CFLAGS} -I../../../../src/include/ -Wall -O3 -static   ../../../../src/lib/m5op_x86.o ../../../../src/lib/m5_mmap.o blackscholes.o -o blackscholes ${LFLAGS} -lm 
error: error.c
	gcc -Wall -O3 -static   error.c -o  error -lm

clean:
	rm -f blackscholes error *.bin 

