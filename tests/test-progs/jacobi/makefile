all: jacobi error

jacobi: jacobi.c
	gcc --static ${CFLAGS} -I../../../../src/include/  -Wall -O3   jacobi.c -c
	gcc --static ${CFLAGS} -I../../../../src/include/  -Wall -O3   jacobi.o ../../../../src/lib/m5op_x86.o  -o  jacobi -lm
error: error.c
	gcc ${CFLAGS} -Wall -O3 --static  error.c -o  error -lm

clean:
	rm -f jacobi error *.out *.bin *.o 

