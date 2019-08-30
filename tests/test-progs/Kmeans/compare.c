#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "kmeans.h"

int      _debug;

float relerror(float golden, float test){
    return fabs( (fabs(golden) - fabs(test))/golden);
}

float abserror(float golden, float test){
    return fabs(fabs(golden) - fabs(test));
}

void compareClusterCenters( char *goldenFilePrefix, char *checkFilePrefix, double *relError, double *absError ){
    char fileName[120];
    float **goldenClustes;
    float **testClusters;
    int i,j;

    int numClusters, numDims;
    strncpy(fileName, goldenFilePrefix, 100);
    strcat(fileName, ".cluster_centres");
    printf("Golden Name %s\n", fileName);
    goldenClustes = file_read(0,fileName, &numClusters, &numDims); 
    check_repeated_clusters(numClusters, numDims, goldenClustes);

    strncpy(fileName, checkFilePrefix, 100);
    strcat(fileName, ".cluster_centres");
    printf("Test Name %s\n", fileName);

    testClusters = file_read(0,fileName, &numClusters, &numDims); 
    check_repeated_clusters(numClusters, numDims, testClusters);

    *relError = 0.0;
    *absError = 0.0;

    for ( i = 0; i < numClusters; i++){
        for ( j = 0 ; j < numDims; j++){
            *relError += relerror(goldenClustes[i][j],testClusters[i][j]);        
            *absError += abserror(goldenClustes[i][j],testClusters[i][j]);        
        }
    }

    *relError= *relError / (double)(numClusters*numDims);
    *absError = *absError / (double)(numClusters*numDims);
    free(goldenClustes[0]);
    free(goldenClustes);
    free(testClusters[0]);
    free(testClusters);

}

int compareMemberShips( char *goldenFilePrefix, char *checkFilePrefix, int numMembers){
    char fileName[120];
    int i,j;
    FILE *golden, *test;
    int count;

    int numClusters, numDims;
    strncpy(fileName, goldenFilePrefix, 100);
    strcat(fileName, ".membership");
    printf("Golden Name %s\n", fileName);
    golden = fopen(fileName, "r");
    if (!golden){
        printf("Could not open %s file \n", fileName);
	printf("Error");
        exit(-1);
    }


    strncpy(fileName, checkFilePrefix, 100);
    strcat(fileName, ".membership");
    printf("Test Name %s\n", fileName);
    test = fopen(fileName, "r");
    if (!test ){
        printf("Could not open %s file \n", fileName);
	printf("Error");
        exit(-1);
    }
    count = 0;
    int correct= 0;
    int gid,tid, goldenCluster, testCluster;
    while ( count < numMembers){
        int items = fscanf(golden,"%d %d\n", &gid, &goldenCluster);
        if ( items != 2 ){
            printf("This should not happen Golden File should always contain correct number of items\n");
	    printf("Error");
            exit(-1);
        }
        items = fscanf(test,"%d %d\n", &tid, &testCluster);
        if ( items != 2 )
            break;

        if ( testCluster == goldenCluster )
            correct++;
        count++;            

    }
    fclose(test);
    fclose(golden);
    return correct;
}

int main(int argc, char *argv[]){

    if (argc != 4){
        printf("%s 'goldenFilePrefix' 'checkFilePrefix' 'number of lines'\n", argv[0]);
        exit(-1);
    }
    
    char *goldenFilePrefix = argv[1];
    char *checkFilePrefix = argv[2];
    int numMembers = atoi(argv[3]);
    int correctMembers;
    double clusterAbsError, clusterRelError;

    compareClusterCenters(goldenFilePrefix, checkFilePrefix, &clusterRelError, &clusterAbsError);
    correctMembers = compareMemberShips(goldenFilePrefix, checkFilePrefix, numMembers);
    printf("%g,%g,%g\n", clusterRelError, clusterAbsError, ((double)correctMembers/(double)numMembers));


    
}
