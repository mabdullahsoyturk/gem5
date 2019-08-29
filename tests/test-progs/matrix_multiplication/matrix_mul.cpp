#include <iostream> 
  
using namespace std; 
  
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
  
// Driver Code 
int main() 
{ 
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
            cout << result[i][j] << " "; 
        } 
        cout << "\n"; 
    } 
  
    return 0; 
}
