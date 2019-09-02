// Copyright (c) 2007 Intel Corp.

// Black-Scholes
// Analytical method for calculating European Options
//
// 
// Reference Source: Options, Futures, and Other Derivatives, 3rd Edition, Prentice 
// Hall, John C. Hull,

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//Precision to use for calculations
#define fptype double 

#define NUM_RUNS 1

typedef struct OptionData_ {
    fptype s;          // spot price
    fptype strike;     // strike price
    fptype r;          // risk-free interest rate
    fptype divq;       // dividend rate
    fptype v;          // volatility
    fptype t;          // time to maturity or option expiration in years 
    //     (1yr = 1.0, 6mos = 0.5, 3mos = 0.25, ..., etc)  
    char OptionType;   // Option type.  "P"=PUT, "C"=CALL
    fptype divs;       // dividend vals (not used in this test)
    fptype DGrefval;   // DerivaGem Reference Value
} OptionData;

OptionData *data;
fptype *prices;
int numOptions;

int    * otype;
fptype * sptprice;
fptype * strike;
fptype * rate;
fptype * volatility;
fptype * otime;
int numError = 0;
int nThreads=1;

const float loge =  0.69314718055995f;
float *pt;
int prec;

#define inv_sqrt_2xPI 0.39894228040143270286








#define __HI(x) *(1+(int*)&x)
#define __LO(x) (*(int*)&x)
#define __HIp(x) *(1+(int*)x)
#define __LOp(x) *(int*)x


static double
one	= 1.0,
    halF[2]	= {0.5,-0.5,},
    huge	= 1.0e+300,
    twom1000= 9.33263618503218878990e-302,     /* 2**-1000=0x01700000,0*/
    o_threshold=  7.09782712893383973096e+02,  /* 0x40862E42, 0xFEFA39EF */
    u_threshold= -7.45133219101941108420e+02,  /* 0xc0874910, 0xD52D3051 */
    ln2HI[2]   ={ 6.93147180369123816490e-01,  /* 0x3fe62e42, 0xfee00000 */
        -6.93147180369123816490e-01,},/* 0xbfe62e42, 0xfee00000 */
        ln2LO[2]   ={ 1.90821492927058770002e-10,  /* 0x3dea39ef, 0x35793c76 */
            -1.90821492927058770002e-10,},/* 0xbdea39ef, 0x35793c76 */
            invln2 =  1.44269504088896338700e+00, /* 0x3ff71547, 0x652b82fe */
            P1   =  1.66666666666666019037e-01, /* 0x3FC55555, 0x5555553E */
            P2   = -2.77777777770155933842e-03, /* 0xBF66C16C, 0x16BEBD93 */
            P3   =  6.61375632143793436117e-05, /* 0x3F11566A, 0xAF25DE2C */
            P4   = -1.65339022054652515390e-06, /* 0xBEBBBD41, 0xC5D26BF1 */
            P5   =  4.13813679705723846039e-08; /* 0x3E663769, 0x72BEA4D0 */

double FAST_EXP( double x )	/* default IEEE double exp */
{
    double y,hi,lo,c,t;
    int k,xsb;
    unsigned hx;
    k = 0;

    hx  = __HI(x);	/* high word of x */
    xsb = (hx>>31)&1;		/* sign bit of x */
    hx &= 0x7fffffff;		/* high word of |x| */

    /* filter out non-finite argument */
    if(hx >= 0x40862E42) {			/* if |x|>=709.78... */
        if(hx>=0x7ff00000) {
            if(((hx&0xfffff)|__LO(x))!=0) 
                return x+x; 		/* NaN */
            else return (xsb==0)? x:0.0;	/* exp(+-inf)={inf,0} */
        }
        if(x > o_threshold) return huge*huge; /* overflow */
        if(x < u_threshold) return twom1000*twom1000; /* underflow */
    }

    /* argument reduction */
    if(hx > 0x3fd62e42) {		/* if  |x| > 0.5 ln2 */ 
        if(hx < 0x3FF0A2B2) {	/* and |x| < 1.5 ln2 */
            hi = x-ln2HI[xsb]; lo=ln2LO[xsb]; k = 1-xsb-xsb;
        } else {
            k  = (int)(invln2*x+halF[xsb]);
            t  = k;
            hi = x - t*ln2HI[0];	/* t*ln2HI is exact here */
            lo = t*ln2LO[0];
        }
        x  = hi - lo;
    } 
    else if(hx < 0x3e300000)  {	/* when |x|<2**-28 */
        if(huge+x>one) return one+x;/* trigger inexact */
    }
    else k = 0;

    /* x is now in primary range */
    t  = x*x;
    c  = x - t*(P1+t*(P2+t*(P3+t*(P4+t*P5))));
    if(k==0) 	return one-((x*c)/(c-2.0)-x); 
    else 		y = one-((lo-(x*c)/(2.0-c))-hi);
    if(k >= -1021) {
        __HI(y) += (k<<20);	/* add k to y's exponent */
        return y;
    } else {
        __HI(y) += ((k+1000)<<20);/* add k to y's exponent */
        return y*twom1000;
    }
    return x;
}



double icsi_log_v2(const float val, register float* const pTable, register const unsigned precision)
{
    /* get access to float bits */
    register const int* const pVal = (const int*)(&val);

    /* extract exponent and mantissa (quantized) */
    register const int exp = ((*pVal >> 23) & 255) - 127;
    register const int man = (*pVal & 0x7FFFFF) >> (23 - precision);

    /* exponent plus lookup refinement */
    return ((double)(exp) + pTable[man]) * loge;
}



void fill_icsi_log_table2(const unsigned precision, float* const   pTable)
{
    /* step along table elements and x-axis positions
     *      (start with extra half increment, so the steps intersect at their midpoints.) */
    float oneToTwo = 1.0f + (1.0f / (float)( 1 <<(precision + 1) ));
    int i;
    for(i = 0;  i < (1 << precision);  ++i )
    {
        // make y-axis value for table element
        pTable[i] = logf(oneToTwo) / loge ;

        oneToTwo += 1.0f / (float)( 1 << precision );
    }
}

float my_sqrt(const float x)  
{
    union
    {
        int i;
        float x;
    } u;
    u.x = x;
    u.i = (1<<29) + (u.i >> 1) - (1<<22); 

    // Two Babylonian Steps (simplified from:)
    // u.x = 0.5f * (u.x + x/u.x);
    // u.x = 0.5f * (u.x + x/u.x);
    u.x =       u.x + x/u.x;
    u.x = 0.25f*u.x + x/u.x;  
    return u.x;
}






fptype CNDF (double InputX ) 
{
    int sign;

    fptype OutputX;
    fptype xInput;
    fptype xNPrimeofX;
    fptype expValues;
    fptype xK2;
    fptype xK2_2, xK2_3;
    fptype xK2_4, xK2_5;
    fptype xLocal, xLocal_1;
    fptype xLocal_2, xLocal_3;

    // Check for negative value of InputX
    if (InputX < 0.0) {
        InputX = -InputX;
        sign = 1;
    } else 
        sign = 0;
    xInput = InputX;

    // Compute NPrimeX term common to both four & six decimal accuracy calcs
    expValues = FAST_EXP(-0.5f * InputX * InputX);
    xNPrimeofX = expValues;
    xNPrimeofX = xNPrimeofX * inv_sqrt_2xPI;

    xK2 = 0.2316419 * xInput;
    xK2 = 1.0 + xK2;
    xK2 = 1.0 / xK2;
    xK2_2 = xK2 * xK2;
    xK2_3 = xK2_2 * xK2;
    xK2_4 = xK2_3 * xK2;
    xK2_5 = xK2_4 * xK2;

    xLocal_1 = xK2 * 0.319381530;
    xLocal_2 = xK2_2 * (-0.356563782);
    xLocal_3 = xK2_3 * 1.781477937;
    xLocal_2 = xLocal_2 + xLocal_3;
    xLocal_3 = xK2_4 * (-1.821255978);
    xLocal_2 = xLocal_2 + xLocal_3;
    xLocal_3 = xK2_5 * 1.330274429;
    xLocal_2 = xLocal_2 + xLocal_3;

    xLocal_1 = xLocal_2 + xLocal_1;
    xLocal   = xLocal_1 * xNPrimeofX;
    xLocal   = 1.0 - xLocal;


    OutputX  = xLocal;
    if (sign) {
        OutputX = 1.0 - OutputX;
    }
    return OutputX;
    return OutputX;
} 


// For debugging
void print_xmm(fptype in, char* s) {
    printf("%s: %f\n", s, in);
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

fptype BlkSchlsEqEuroNoDiv( fptype sptprice,
        fptype strike, fptype rate, fptype volatility,
        fptype time, int otype, float timet )
{
    fptype OptionPrice;

    // local private working variables for the calculation
    fptype xRiskFreeRate;
    fptype xVolatility;
    fptype xTime;
    fptype xSqrtTime;

    fptype logValues;
    fptype xLogTerm;
    fptype xD1; 
    fptype xD2;
    fptype xPowerTerm;
    fptype xDen;
    fptype FutureValueX;
    fptype NofXd1;
    fptype NofXd2;
    fptype NegNofXd1;
    fptype NegNofXd2;    

    xRiskFreeRate = rate;
    xVolatility = volatility;

    xTime = time;

    logValues = icsi_log_v2(sptprice / strike , pt, prec);


    xLogTerm = logValues;


    xPowerTerm = xVolatility * xVolatility;
    xPowerTerm = xPowerTerm * 0.5;
    xD1 = xRiskFreeRate + xPowerTerm;
    xD1 = xD1 * xTime;
    xD1 = xD1 + xLogTerm;


    xSqrtTime = my_sqrt(xTime);


    xDen = xVolatility * xSqrtTime;
    xD1 = xD1 / xDen;
    NofXd1 = CNDF( xD1 );
    xD2 = xD1 -  xDen;
    NofXd2 = CNDF( xD2 );
    FutureValueX = strike * ( FAST_EXP ( -(rate)*(time) ) );          

    if (otype == 0) {            
        OptionPrice = (sptprice * NofXd1) - (FutureValueX * NofXd2);
    } else { 
        NegNofXd1 = (1.0 - NofXd1);
        NegNofXd2 = (1.0 - NofXd2);
        OptionPrice = (FutureValueX * NegNofXd2) - (sptprice * NegNofXd1);
    }


    return OptionPrice;
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

int bs_thread(void *tid_ptr) {
    int i;
    fptype price;
    int tid = *(int *)tid_ptr;
    int start = tid * (numOptions / nThreads);
    int end = start + (numOptions / nThreads);
    for (i=start; i<end; i++) {
        /* Calling main function to calculate option value based on 
         * Black & Sholes's equation.
         */
        price = BlkSchlsEqEuroNoDiv( sptprice[i], strike[i],
                rate[i], volatility[i], otime[i], 
                otype[i], 0);
        prices[i] = price;

    }
    return 0;
}

int main (int argc, char **argv)
{
    FILE *file;
    int i;
    int loopnum;
    fptype * buffer;
    int * buffer2;
    int rv;

    if (argc != 3)
    {
        printf("Usage:\n\t%s <inputFile> <outputFile>\n", argv[0]);
	printf("Error");
        exit(1);
    }
    char *inputFile = argv[1];
    char *outputFile = argv[2];


    //Read input data from file
    file = fopen(inputFile, "r");
    if(file == NULL) {
        printf("Error: Unable to open file `%s'.\n", inputFile);
        exit(1);
    }
    rv = fscanf(file, "%i", &numOptions);
    if(rv != 1) {
        printf("Error: Unable to read from file `%s'.\n", inputFile);
        fclose(file);
        exit(1);
    }
    if(nThreads > numOptions) {
        printf("WARNING: Not enough work, reducing number of threads to match number of options.\n");
        nThreads = numOptions;
    }


    // alloc spaces for the option data
    data = (OptionData*)malloc(numOptions*sizeof(OptionData));
    prices = (fptype*)malloc(numOptions*sizeof(fptype));
    for ( loopnum = 0; loopnum < numOptions; ++ loopnum )
    {
        rv = fscanf(file, "%lf %lf %lf %lf %lf %lf %c %lf %lf", &data[loopnum].s, &data[loopnum].strike, &data[loopnum].r, &data[loopnum].divq, &data[loopnum].v, &data[loopnum].t, &data[loopnum].OptionType, &data[loopnum].divs, &data[loopnum].DGrefval);
        if(rv != 9) {
            printf("Error: Unable to read from file `%s'.\n", inputFile);
            fclose(file);
            exit(1);
        }
    }
    rv = fclose(file);
    if(rv != 0) {
        printf("Error: Unable to close file `%s'.\n", inputFile);
        exit(1);
    }

    printf("Num of Options: %d\n", numOptions);
    printf("Num of Runs: %d\n", NUM_RUNS);

#define PAD 256
#define LINESIZE 64

    buffer = (fptype *) malloc(5 * numOptions * sizeof(fptype) + PAD);
    sptprice = (fptype *) (((unsigned long long)buffer + PAD) & ~(LINESIZE - 1));
    strike = sptprice + numOptions;
    rate = strike + numOptions;
    volatility = rate + numOptions;
    otime = volatility + numOptions;

    buffer2 = (int *) malloc(numOptions * sizeof(fptype) + PAD);
    otype = (int *) (((unsigned long long)buffer2 + PAD) & ~(LINESIZE - 1));

    for (i=0; i<numOptions; i++) {
        otype[i]      = (data[i].OptionType == 'P') ? 1 : 0;
        sptprice[i]   = data[i].s;
        strike[i]     = data[i].strike;
        rate[i]       = data[i].r;
        volatility[i] = data[i].v;    
        otime[i]      = data[i].t;
    }

    printf("Size of data: %ld\n", numOptions * (sizeof(OptionData) + sizeof(int)));
    
    prec = 20;
    int size = pow(2,prec);
    pt= (float*) malloc (sizeof(float)*size);
    fill_icsi_log_table2(prec,pt);

    i =0;
    bs_thread((void*) &i );

    file = fopen(outputFile, "wb");
    if(file == NULL) {
        printf("Error: Unable to open file `%s'.\n", outputFile);
        exit(1);
    }
    
    int res = fwrite(&numOptions,sizeof(int),1,file);

    if(res != 1) {
        printf("Error: Unable to write to file `%s'.\n", outputFile);
        fclose(file);
        exit(1);
    }
    
    res = fwrite(prices, sizeof(fptype), numOptions, file);

    if(res != numOptions) {
        printf("Error: Unable to write to file `%s'.\n", outputFile);
        fclose(file);
        exit(1);
    }
    
    rv = fclose(file);
    if(rv != 0) {
        printf("Error: Unable to close file `%s'.\n", outputFile);
        exit(1);
    }

    return 0;
}

