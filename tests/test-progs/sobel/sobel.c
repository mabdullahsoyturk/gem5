#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#ifdef GEMFI
#include <m5op.h>
#endif

#define SIZE	512

/* The arrays holding the input image, the output image and the output used *
 * as golden standard. The luminosity (intensity) of each pixel in the      *
 * grayscale image is represented by a value between 0 and 255 (an unsigned *
 * character). The arrays (and the files) contain these values in row-major *
 * order (element after element within each row and row after row. 			*/
unsigned char input[SIZE*SIZE], output[SIZE*SIZE];


#define SQRT(res) \
  u.x = (float) res; \
u.i = (1<<29) + (u.i >> 1) - (1<<22); \
u.x =       u.x + res/u.x; \
u.x = 0.25f*u.x + res/u.x; \
res = u.x;


#define CLIP(val,out) \
  value = val; \
r = (value >> 15); \
s = !!(value & 0x7f00) * 0xff; \
v = (value & 0xff); \
*out =  (v | s) & ~r; 

#define SOBELX(y,x) \
  sx = (int ) input[(y-1)*SIZE+ x+1] + 2*(int)input[y*SIZE+ x+1] + (int )input[(y+1)*SIZE+ x+1] - \
      (int )input[(y-1)*SIZE+ x-1] - (int )2*input[y*SIZE+ x-1] - (int )input[(y+1)*SIZE+ x-1]; \
      sx=sx*sx;

#define SOBELY(y,x) \
  sy = (int)input[(y+1)*SIZE+ x-1] + 2*(int)input[(y+1)*SIZE+ x] + (int)input[(y+1)*SIZE+ x+1] - \
       (int)input[(y-1)*SIZE+ x-1] - (int)2*input[(y-1)*SIZE+ x] - (int)input[(y-1)*SIZE+x+1]; \
       sy = sy*sy;

/* The main computational function of the program. The input, output *
 * arguments are pointers to the arrays used to store the input   *
 * image, the output produced by the algorithm	*/
void sobel(unsigned char *input, unsigned char *output)
{
  int i, j;
  int sx,sy;
  int res;
  short value;
  unsigned char r,s,v;
  union
  {
    int i;
    float x;
  } u;

  for (j=1; j<SIZE-1; j++) {
    for (i=1; i<SIZE-1; i++ ) {
      SOBELX(j,i);
      SOBELY(j,i);
      res = sy+sx; 
      SQRT(res);
      CLIP(res,&output[j*SIZE + i])
    }

  }


  return;
}


int main(int argc, char* argv[])
{
  int i;
  FILE *f_in,*f_out;

  if ( argc != 3 ){
      printf("%s 'input file' 'output file'\n",argv[0]);
      printf("Error");
      exit(-1);
  }
  char *inputFile = argv[1];
  char *outputFile= argv[2];

  memset(output, 0, SIZE*sizeof(unsigned char));
  memset(&output[SIZE*(SIZE-1)], 0, SIZE*sizeof(unsigned char));
  for (i = 1; i < SIZE-1; i++) {
    output[i*SIZE] = 0;
    output[i*SIZE + SIZE - 1] = 0;
  }

  f_in = fopen(inputFile, "r");
  if (f_in == NULL) {
    printf("Error:File %s not found\n", inputFile);
    exit(1);
  }

  f_out = fopen(outputFile, "wb");
  if (f_out == NULL) {
    printf("Error:File %s could not be created\n", outputFile);
    fclose(f_in);
    exit(1);
  }  

  int size = fread(input, sizeof(unsigned char), SIZE*SIZE, f_in);
  if ( size != SIZE * SIZE ){
      printf("Error:Could not read entire file ... exit\n");
      fclose(f_in);
      fclose(f_out);
      exit(-1);
  }
  fclose(f_in);

  sobel(input, output );
  size = fwrite(output, sizeof(unsigned char), SIZE * SIZE, f_out);
  if ( size != SIZE * SIZE ){
      printf("Error:Could not write entire file ... exit\n");
      fclose(f_in);
      fclose(f_out);
      exit(-1);
  }
  printf("PSNR of original Sobel and computed Sobel image: %s\n", outputFile);
}


