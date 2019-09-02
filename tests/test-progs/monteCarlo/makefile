all: monte_carlo calc_errors

monte_carlo: monte_carlo.c  
	gcc --static ${CFLAGS} -I../../../../src/include/ -Wall -O3   monte_carlo.c -c
	gcc --static ${CFLAGS} -Wall -O3 monte_carlo.o  ../../../../src/lib/m5op_x86.o ../../../../src/lib/m5_mmap.o -o  monte_carlo -lm
calc_errors: calc_errors.c
	gcc ${CFLAGS} -Wall -O3 --static  calc_errors.c -o  calc_errors -lm

clean:
	rm -f monte_carlo calc_errors *.o *.bin

