#include <stdio.h> 
  
#define N 512 

void multiply(int matrix1[][N], int matrix2[][N], int result[][N]) 
{ 
    int i, j, k; 
    for (i = 0; i < N; i++) 
    { 
        for (j = 0; j < N; j++) 
        { 
            result[i][j] = 0; 
            for (k = 0; k < N; k++) {
                result[i][j] += matrix1[i][k] * matrix2[k][j]; 
            }
        } 
    } 
} 
  
int main(int argc, char **argv) 
{ 
    if (argc != 2)
    {
        printf("Usage:\n\t%s <outputFile>\n", argv[0]);
        exit(1);
    }

    FILE* output_file = fopen(argv[1], "w");

    int i, j; 
    int result[N][N];

    int matrix1[N][N];
    int matrix2[N][N];

    for(i = 0; i < N; i++) 
    {
        for(j = 0; j < N; j++) 
        {
            matrix1[i][j] = i+1;
            matrix2[i][j] = i+1;
        }
    }
  
    multiply(matrix1, matrix2, result); 
  
    for (i = 0; i < N; i++) 
    { 
        for (j = 0; j < N; j++) {
            printf("%d ", result[i][j]); 
        } 
        printf("\n");
    } 
  
    return 0; 
}
