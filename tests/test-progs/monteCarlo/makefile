all: monte_carlo calc_errors

monte_carlo: monte_carlo.c 
	gcc ${CFLAGS} -Wall -O3 --static  monte_carlo.c -o  monte_carlo -lm
calc_errors: calc_errors.c
	gcc ${CFLAGS} -Wall -O3 --static  calc_errors.c -o  calc_errors -lm

clean:
	rm -f monte_carlo calc_errors *.bin

