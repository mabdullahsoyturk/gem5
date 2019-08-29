all: matrix_mul

matrix_mul: matrix_mul.cpp
	g++ -Wall -O3 -static matrix_mul.cpp -o matrix_mul -lm

clean:
	rm -f *.o matrix_mul
