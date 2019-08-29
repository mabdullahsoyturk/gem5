#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double relerror(double golden, double test){
    if (golden != 0.0 )
        return fabs( (fabs(golden) - fabs(test))/golden);
    else
        return fabs(fabs(golden) - fabs(test));
}

double abserror(double golden, double test){
    return fabs(fabs(golden) - fabs(test));
}

int main(int argc, char *argv[]){
    if (argc != 3 ){
        printf("%s 'correct output file' 'Testing output file')\n", argv[0]);
    }
    long i;
    double relErr = 0.0;
    double absErr = 0.0;

    char *goldenFile = argv[1];
    char *testFile = argv[2];
    FILE *golden, *test;
    golden = fopen(goldenFile, "r");
    test= fopen(testFile, "r");

    if ( !(golden && test)){
        printf("Could not open files \n");
        exit(-1);
    }

    int goldenSize;
    int testSize;

    int res = fread(&goldenSize, sizeof(int), 1,  golden);
    if (res != 1) {
        printf("Could not read total number of bytes \n");
        printf("Rel Error is 1.0 AbsError is 1.0 \n");
        return 1;
    }

    res = fread(&testSize,  sizeof(int), 1, test);
    if (res != 1 ){
        printf("Could not read total number of bytes \n");
        printf("Rel Error is 1.0 AbsError is 1.0 \n");
        return 1;
    }

    if ( testSize != goldenSize ){
        printf("Files differ in size\n");
        printf("Rel Error is 1.0 AbsError is 1.0 \n");
        return 1;
    }


    for ( i = 0 ; i < testSize; i++){
        double val1, val2;
        res = fread(&val1, sizeof(double), 1,  golden);
        if (res != 1 ){
            printf("Could not read total number of bytes \n");
            printf("Rel Error is 1.0 AbsError is 1.0 \n");
            return 1;
        }
        res = fread(&val2, sizeof(double), 1, test);
        if (res != 1 ){
            printf("Could not read total number of bytes \n");
            printf("Rel Error is 1.0 AbsError is 1.0 \n");
            return 1;
        }

        relErr += relerror(val1,val2);        
        absErr += abserror(val1, val2);        
        if (feof(golden) || feof(test) ){
            printf("File terminated to early should not happen\n");
            printf("Rel Error is 1.0 AbsError is 1.0 \n");
            fclose(golden);
            fclose(test);
        }
    }

    printf("Rel Error is %g AbsError is %g \n", relErr/(double)(goldenSize), absErr/(double) (goldenSize));
    fclose(golden);
    fclose(test);

}
