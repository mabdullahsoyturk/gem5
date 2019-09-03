#include <assert.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#define TASK_GRAIN 16
#define USEC_TO_MILLISEC 1000000.0

#define N 512
#define SIZE N
long widthInBlocks, heightInBlocks;


int quant_table[8*8] = 
{
    16, 11, 10, 16, 24, 40, 51, 61, 
    12, 12, 14, 19, 26, 58, 60, 55,
    14, 13, 16, 24, 40, 57, 69, 56,
    14, 17, 22, 29, 51, 87, 80, 82,
    18, 22, 37, 56, 68, 109, 103, 77,
    24, 35, 55, 64, 81, 104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101,
    72, 92, 95, 98, 112, 100, 103, 99
};

double COS[] = { 1.0,0.98078528040323043,0.92387953251128674,0.83146961230254524,0.70710678118654757,0.55557023301960229,0.38268343236508984,0.19509032201612833,1.0,0.83146961230254524,0.38268343236508984,-0.19509032201612819,-0.70710678118654746,-0.98078528040323043,-0.92387953251128685,-0.55557023301960218,1.0,0.55557023301960229,-0.38268343236508973,-0.98078528040323043,-0.70710678118654768,0.1950903220161283,0.92387953251128652,0.83146961230254546,1.0,0.19509032201612833,-0.92387953251128674,-0.55557023301960218,0.70710678118654735,0.83146961230254546,-0.38268343236508989,-0.98078528040323065,1.0,-0.19509032201612819,-0.92387953251128685,0.55557023301960184,0.70710678118654768,-0.83146961230254512,-0.38268343236509056,0.98078528040323043,1.0,-0.55557023301960196,-0.38268343236509034,0.98078528040323043,-0.70710678118654668,-0.19509032201612803,0.92387953251128674,-0.83146961230254501,1.0,-0.83146961230254535,0.38268343236509,0.19509032201612878,-0.70710678118654713,0.98078528040323065,-0.92387953251128641,0.55557023301960151,1.0,-0.98078528040323043,0.92387953251128652,-0.83146961230254512,0.70710678118654657,-0.55557023301960151,0.38268343236508956,-0.19509032201612858 };

double C[] = { 0.70710678118654746,1,1,1,1,1,1,1 };

unsigned char*idct;
double *dct;
unsigned char *pic;



int my_round(double a) {
    int s = a >= 0.0 ? 1 : -1;
    return (int)(a+s*0.5);
}


void idct_task(long taskRowStart, long workStart){
    long x, y, i, j, r, c;
    long workEnd;
    double sum = 0;
    r = taskRowStart;
    c = workStart;
    if ( widthInBlocks-workStart > TASK_GRAIN )
        workEnd = workStart+TASK_GRAIN;
    else
        workEnd = widthInBlocks; 

    for ( c = workStart; c<workEnd; ++c )
    {
        for ( i=0; i<8; ++i )
            for ( j=0; j<8; ++j)
            {
                sum = 0.0;
                for (x = 0; x < 8; x++){
                    for (y = 0; y < 8; y++)
                        sum += C[x] * C[y] * dct[(r * 8 + x)*widthInBlocks*8+ c * 8 + y] * COS[i*8+x] * COS[j*8+y];
                }
                sum = sum*0.25+128;
                idct[(r*8+i)*widthInBlocks*8+ c*8+ j] = (unsigned char) sum; 
            }
    }
}

void IDCT() 
{
    long r, c;

    for (r = 0; r < heightInBlocks; r++)
        for (c = 0; c < widthInBlocks; c+=TASK_GRAIN)
        { 
            idct_task(r, c);
        }

}

double MSE_PSNR(unsigned char *golden, unsigned char* computed) 
{
    int r, c;
    double MSE = 0, PSNR;

    for (r = 0; r < N ; r++){
        for (c = 0; c < N; c++){
            MSE += (golden[r*N+c] - computed[r*N+c]) * (golden[r*N+c] - computed[r*N+c]);
        }
    }

    MSE /= (N * N);
    PSNR = 10 * log10(255 * 255 / MSE);
    return PSNR;
}



int
main(int argc, char* argv[]) {
    long r, c;
    double psnr;
    FILE *in;

    if ( argc != 4 ) {
        printf("usage %s 'input file' 'golden file' 'output file' \n", argv[0]);
        return (0);
    }

    widthInBlocks = 64;
    heightInBlocks = 64;
    pic = (unsigned char*) malloc(sizeof(unsigned char)*widthInBlocks* heightInBlocks*64);
    dct = (double*) malloc(sizeof(double)*widthInBlocks*heightInBlocks*64);
    idct = (unsigned char *) malloc(sizeof(unsigned char)*widthInBlocks*heightInBlocks*64);

    in = fopen(argv[1], "rb");
    unsigned char temp;

    for (r = 0; r < N; r++){
        for (c = 0; c < N; c++){
            assert(fscanf(in, "%c", &temp));
            dct[r*SIZE + c] = (double) temp;
        }
    }
    fclose(in);

    in = fopen(argv[2], "rb");
    for (r = 0; r < N; r++){
        for (c = 0; c < N; c++){
            assert(fscanf(in, "%c", &temp));
            pic[r*SIZE + c] = (double) temp;
        }
    }
    fclose(in);

    IDCT();
    psnr = MSE_PSNR(pic,idct);
    printf("PSNR is %g\n", psnr);

    FILE *out = fopen(argv[3],"wb");
    for (r = 0; r < N; r++)
        for (c = 0; c < N; c++)
            fputc(idct[r*N+c], out);
    fclose(out);

    return 0;
}
