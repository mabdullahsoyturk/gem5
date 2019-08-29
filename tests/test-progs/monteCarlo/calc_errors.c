#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char* argv[])
{
    FILE *f1, *f2;
    long nz1, nz2;
    long num1, num2, dur1, dur2,i;
    double x, y;
    double err, tot, err2;
    err2= 0.0;
    err = 0.0;
    tot = 0.0;


    if ( argc!= 3 ) {
        printf("usage: %s ''golden'' ''aprox''\n", argv[0]);
        return 1;
    }

    f1 = fopen(argv[1], "rb");
    f2 = fopen(argv[2], "rb");

    if ( !f1 || !f2 ) {
        printf("cannot access files\n");
        return 2;
    }

    assert(fread(&num1, sizeof(long), 1, f1)); assert(fread(&num2, sizeof(long), 1, f2));
    if ( num1 != num2 ) {
        printf("file sizes do not match\n");
        fclose(f1);
        fclose(f2);
        return 3;
    }

    assert(fread(&dur1, sizeof(long), 1, f1)); assert(fread(&dur2, sizeof(long), 1, f2));
    nz1=0;
    nz2=0;

    for (i=0; i<num1; ++i )
    {

        assert(fread(&x, sizeof(double), 1, f1));
        assert(fread(&y, sizeof(double), 1, f2));
        if ( x!=0.0) 
            nz1++;
        if ( y!=0.0)
            nz2++;
        x = fabs(x);
        tot += x;
        y = fabs(y);
        x -= y;
        err2 += fabs(x);
        x *= x;
        err += x;
    }
    err  /= (double)num1;
    err2 /=tot;
    //	printf("MSE=%G\nRelativeError=%G\nTimeExact=%f\nTimeApprox=%g\nSpeedup=%g\n", err, err2, (double)(dur1)/1000000.0, (double)dur2/1000000.0, (float)dur1/dur2);
    //  printf("NZ1=%ld out of %ld\nNZ2=%ld\n", nz1, num1, nz2);
    printf("%G %G", err, err2); 
    fclose(f1);
    fclose(f2);

    return 0;
}
