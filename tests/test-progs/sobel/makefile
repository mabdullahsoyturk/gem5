all: sobel psnr

sobel: sobel.c
	gcc -Wall -O3 -static  sobel.c -o  sobel -lm
psnr: psnr.c
	gcc -Wall -O3 -static  psnr.c -o  psnr -lm

clean:
	rm -f sobel psnr *.out 

