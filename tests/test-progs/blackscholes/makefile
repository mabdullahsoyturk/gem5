all: blackscholes error

blackscholes: blackscholes.c
	gcc -Wall -O3 -static  blackscholes.c -o  blackscholes -lm
error: error.c
	gcc -Wall -O3 -static   error.c -o  error -lm

clean:
	rm -f blackscholes error *.bin 

