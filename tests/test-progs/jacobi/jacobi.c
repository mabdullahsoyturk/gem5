#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <sys/time.h>



double *construct_jacobi_matrix(int diagonally_dominant, int size);
double *construct_b(int size);
double jacobi(double *A, double *x, double *x1,  double *b, double* y,
        long int size, double itol, unsigned int *iters);

double *construct_right(int size, double *mat, double* sol);


long my_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000000+tv.tv_usec;
}

double *construct_solution(int size) {
    int j;
    double	*ret = (double*) malloc(sizeof(double)*size);

    assert(ret);
    for ( j=0; j<size; ++j ) {
        ret[j] = fmod((double)(rand()-RAND_MAX/2.0),size);
    }

    return ret;
}

double *construct_right(int size, double *A, double *sol)
{
    int i, j;
    double *ret = (double*) malloc(sizeof(double)*size);
    double s;

    for ( i=0; i<size; ++i )
    {
        s = 0;
        for ( j=0; j<size; ++j )
        {
            s += A[i*size+j] * sol[j];
        }
        ret[i] = s;
    }
    return ret;
}

// This is just a simple way to make sure that a matrix
// which consists of random numbers is diagonally dominant
// This feature is optional, set call with diagonally_dominant!=0 in order
// to use it during the matrix generation.
double *construct_jacobi_matrix(int diagonally_dominant, int size) {
    double *ret;
    int i, j;
    unsigned long int sum;
    int sure;

    sure = size/10;
    if ( sure < 5 )
        sure = 5;

    ret = (double*) malloc(sizeof(double)*size*size);
    assert(ret);
    for ( i=0; i<size; ++i ) {
        for ( j=0; j<i; ++j ) {
            if ( ( fabs(j-i) < sure ) || rand()%100 < 10)
                ret[i*size+j] = (double)(rand())-RAND_MAX/2.0;
            else
                ret[i*size+j] = 0.0;
            ret[i*size+j] = fmod(ret[i*size+j],1000.0);
            ret[i*size+j] /= 100.0;
        }
    }
    // Sum the absolute values of the rows and add something (positive) random to 
    // in order for it to be stored in the diagonal the sum

    if ( diagonally_dominant ) {
        for ( i=0; i<size; ++i ) {
            sum = 0;
            for ( j=0; j<i; ++j )  {
                sum+= abs(ret[i*size+j]);
            }
            for ( j=i+1; j<size; ++j ) {
                sum+= abs(ret[i*size+j]);
            }
            sum += rand()/(double)100+1;
            // keep the sign of the diagonal
            ret[i*size+i] = (ret[i*size+1] < 0 )? -sum : sum;
        }
    }

    return ret;
}

// This is the actual benchmark kernel
double jacobi(double *A, double *x, double *x1,  double *b, double* y,
        long int size, double itol, unsigned int *iters)
{
    int iter, i, j;
    double dif,  s, t;
    dif = 10000;

    for (i=0; i<size; ++i )
    {
        x[i] = 1.0/A[i*size+i];
    }

    for ( iter = 0; iter<*iters && (  dif > itol ) ; ++iter ) {
        for ( i=0; i<size; i++ ) {
            s = 0.0;
            for ( j = 0; j<i; ++j ) {
                s += A[i*size+j] * x[j];
            }
            for ( j =i+1; j<size; ++j ) {
                s += A[i*size+j] * x[j];
            }
            s = ( (double) b[i] - s ) / (double)A[i*size+i];
            x1[i] = s;
        }

        dif = 0.0;
        for ( i=0; i<size; i++ ) {
            t = fabs(y[i] - x1[i] );
            x[i] = x1[i];
            if ( t>dif ) {
                dif = t;
            }
        }
    }
    *iters = iter;
    return dif;
}

int main(int argc, char* argv[]) {
    double *x, itol, *y, *x1;
    double *mat, *b;
    long N;
    char *endptr;
    int diagonally_dominant;
    unsigned int iters;
    double diff;
    int i;
    long  _seed;
    FILE* output;

    if (argc != 6){
        printf("usage ./jacobi 'long:N' 'double:itol' 'bool:diagonally_dominant' 'long:max_iters'  'string:output_file'\n");
	printf("Error");
        return(1);
    }

    N = strtol(argv[1], &endptr, 10);

    if ( *endptr != '\0' ) {
        printf("Error:Invalid input. First argument (N) must be a long represented"
                " in base 10.\n");
        return 1;
    }
    itol = strtod(argv[2], &endptr);
    if ( *endptr != '\0' ) {
        printf("Error:Invalid input. Second argument (itol) must be a double represented"
                " in base 10.\n");
        return 1;
    }

    diagonally_dominant = strtol(argv[3], &endptr, 10);
    if ( *endptr != '\0' ) {
        printf("Error:Invalid input. Third argument (diagonally_dominant) must be a"
                " long represented in base 10.\n");
        return 1;
    }

    iters = strtod(argv[4], &endptr);
    if ( *endptr != '\0' ) {
        printf("Error:Invalid input. Fourth argument (max_iters) must be a double"
                " represented in base 10.\n");
        return 1;
    }

    assert(diagonally_dominant>=0);
    x = (double*) calloc(N,sizeof(double));
    x1 = (double*) calloc(N, sizeof(double));
    assert(x1);
    assert(x);
    _seed = 20 ;
    srand(_seed);

    y = construct_solution(N);
    assert(y);
    mat = construct_jacobi_matrix(diagonally_dominant, N);
    b = construct_right(N, mat, y);

    diff = jacobi(mat, x, x1, b, y, N, itol, &iters);

    printf("Converged after %d iterations, solution: %g\n", iters,diff);


    output = fopen(argv[5] , "wb");
    fwrite(&N, sizeof(long), 1, output);
    for ( i=0; i<N; ++i )
    {
        fwrite(x+i, sizeof(double), 1, output);
        fwrite(y+i, sizeof(double), 1, output);
    }

    fclose(output);
    free(x1);
    free(b);
    free(x);
    free(y);
    free(mat);
    return 0;
}

