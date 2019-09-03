#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define N 512

double COS[8][8], C[8];
unsigned char pic[N][N];
double dct[N][N], idct[N][N];
double golden[N][N];

void init() {
    int i, j;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++)
            COS[i][j] = cos((2 * i + 1) * j * acos(-1) / 16.0);
        if (i) C[i] = 1;
        else C[i] = 1 / sqrt(2);
    }
}

void DCT() {
    int r, c, i, j, x, y;
    for (r = 0; r < 64; r++)
        for (c = 0; c < 64; c++)
            for (i = 0; i < 8; i++)
                for (j = 0; j < 8; j++) {
                    double sum = 0;
                    for (x = 0; x < 8; x++)
                        for (y = 0; y < 8; y++)
                            sum += (pic[r * 8 + x][c * 8 + y] - 128) * COS[x][i] * COS[y][j];
                    sum *= C[i] * C[j] * 0.25;
                    dct[r * 8 + i][c * 8 + j] = sum;
                }
}

void IDCT() {
    int r, c, i, j, x, y;
    for (r = 0; r < 64; r++)
        for (c = 0; c < 64; c++)
            for (i = 0; i < 8; i++)
                for (j = 0; j < 8; j++) {
                    double sum = 0;
                    for (x = 0; x < 8; x++)
                        for (y = 0; y < 8; y++)
                            sum += C[x] * C[y] * dct[r * 8 + x][c * 8 + y] * COS[i][x] * COS[j][y];
                    sum *= 0.25;
                    sum += 128;
                    idct[r * 8 + i][c * 8 + j] = sum;
                }
}

void quantization() {
    int table[8][8] = {
        {16, 11, 10, 16, 24, 40, 51, 61,},
        {12, 12, 14, 19, 26, 58, 60, 55,},
        {14, 13, 16, 24, 40, 57, 69, 56,},
        {14, 17, 22, 29, 51, 87, 80, 82,},
        {18, 22, 37, 56, 68, 109, 103, 77,},
        {24, 35, 55, 64, 81, 104, 113, 92,},
        {49, 64, 78, 87, 103, 121, 120, 101,},
        {72, 92, 95, 98, 112, 100, 103, 99},
    };
    int r, c, i, j;
    for (r = 0; r < 64; r++)
        for (c = 0; c < 64; c++)
            for (i = 0; i < 8; i++)
                for (j = 0; j < 8; j++) {
                    dct[r * 8 + i][c * 8 + j] = round(dct[r * 8 + i][c * 8 + j] / table[i][j]);
                    dct[r * 8 + i][c * 8 + j] = dct[r * 8 + i][c * 8 + j] * table[i][j];
                }
}

double MSE() {
    double MSE = 0;
    int r, c;
    for (r = 0; r < N; r++)
        for (c = 0; c < N; c++) {
            MSE += (pic[r][c] - idct[r][c]) * (pic[r][c] - idct[r][c]);
        }
    MSE /= (512 * 512);
    double PSNR = 10 * log10(255 * 255 / MSE);
    return PSNR;
}

int main(int argc, char *argv[]) {
    int i,j;
    if (argc != 4){
        printf("%s 'input picture' 'golden output' 'test output'  \n",argv[0]);
        exit(-1);
    }

    char *inpFile = argv[1];
    char *goldenFile= argv[2];
    char *testFile = argv[3];

    FILE *in = fopen(inpFile, "r");
    if (!in){
        printf("Could Not Open File %s\n",inpFile);
        exit(-1);
    }

    if (fread(pic, sizeof(unsigned char), N*N,in) != N*N){
        printf("Could Not write all bytes\n");
        exit(-1);
    }

    fclose(in);

    FILE *goldenF = fopen(goldenFile, "r");
    if (!goldenF){
        printf("Could Not Open File %s\n",goldenFile);
        exit(-1);
    }

    if (fread(golden, sizeof(double), N*N,goldenF) != N*N){
        printf("Could Not write all bytes\n");
        exit(-1);
    }

    fclose(goldenF);

    FILE *testF= fopen(testFile, "r");
    if (!testF){
        printf("Could Not Open File %s\n",testFile);
        exit(-1);
    }

    if (fread(dct, sizeof(double), N*N,testF ) != N*N){
        printf("Could Not write all bytes\n");
        exit(-1);
    }

    fclose(testF);
    int differ = 0; 
    for ( i = 0; i < N; i++)
        for ( j = 0 ; j < N; j++){
            if ( (*(long *) &dct[i][j]) != (* (long *) &golden[i][j])){
                differ=1;
                break;
            }
        }

    if ( !differ ){
        printf("Correct, %lf\n",1.0/0.0); 
        return 1;
    }

    init();
    IDCT();
    printf("SDC, %lf\n", MSE());
    return 0;
}
