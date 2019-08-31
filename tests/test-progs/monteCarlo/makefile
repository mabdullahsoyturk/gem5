all: monte_carlo calc_errors

monte_carlo: monte_carlo.c myMath.c 
	gcc --static ${CFLAGS} -Wall -O3   monte_carlo.c -c
	gcc --static ${CFLAGS} -Wall -O3   myMath.c  -c 
	gcc --static ${CFLAGS} -Wall -O3   myMath.o monte_carlo.c -o  monte_carlo -lm
calc_errors: calc_errors.c
	gcc ${CFLAGS} -Wall -O3 --static  calc_errors.c -o  calc_errors -lm

clean:
	rm -f monte_carlo calc_errors *.o *.bin

