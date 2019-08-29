#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 512

unsigned char output[SIZE*SIZE], golden[SIZE*SIZE];

int main(int argc, char *argv[]){
    double PSNR,t;
    int i,j;
    FILE *f_in, *f_golden;

    if ( argc != 3 ){
        printf("%s 'output file' 'golden file'\n",argv[0]);
        exit(-1);
    }
    char *inputFile = argv[1];
    char *goldenFile = argv[2];

    f_in = fopen(inputFile, "r");
    if (f_in == NULL) {
        printf("File %s  not found\n", inputFile);
        exit(1);
    }

    f_golden = fopen(goldenFile, "r");
    if (f_golden== NULL) {
        printf("File %s could not be created\n", goldenFile);
        fclose(f_in);
        exit(1);
    } 

    int size = fread(output, sizeof(unsigned char), SIZE*SIZE, f_in);
    if (size != SIZE * SIZE){
        printf(" I could not read entire file\n");
        fclose(f_in);
        fclose(f_golden);
        exit(-1);
    }

    size = fread(golden, sizeof(unsigned char), SIZE*SIZE, f_golden);
    if (size != SIZE * SIZE){
        printf(" I could not read entire file\n");
        fclose(f_in);
        fclose(f_golden);
        exit(-1);
    }

    fclose(f_in);
    fclose(f_golden);

    PSNR=0.0; 
    for (i=1; i<SIZE-1; i++) {
        for ( j=1; j<SIZE-1; j++ ) {
            t = output[i*SIZE+j] - golden[i*SIZE+j];
            t = t*t;
            PSNR += t;
        }
    }

    PSNR /= (double)(SIZE*SIZE);
    PSNR = 10*log10(65536/PSNR);
    printf("PSNR of original Sobel and computed Sobel image: %g\n", PSNR);


}
