all: matrix_mul

matrix_mul: matrix_mul.c
	gcc -Wall -O3 -static matrix_mul.c -o matrix_mul -lm

clean:
	rm -f *.o matrix_mul
