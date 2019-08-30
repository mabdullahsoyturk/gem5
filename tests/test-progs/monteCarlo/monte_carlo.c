#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <sys/time.h>
#include <stdlib.h>

double rand_uniform() {
	double r = rand()/(double)RAND_MAX;
	return r;
}

void update_y_2D(double x[], double y[], double d)
{
	double angle = rand_uniform()*(2.*M_PI);
	double rad;
	do {rad = rand_uniform()*d;} while (((4.*rad)/(d*d))*log(d/rad) < rand_uniform()*(4./(M_E*d)));
	y[0] = x[0] + rad*cos(angle);
	y[1] = x[1] + rad*sin(angle);
}

void update_x_2D(double x[], double d)
{
	double angle = rand_uniform()*(2.*M_PI);
	x[0] += d*cos(angle);
	x[1] += d*sin(angle);
}

double estimate_point_2D(double x[])
{
	return x[0]*x[1]*(x[0]-1.)*(x[1]-1.);
}

double estimate_walk_2D(double x[], double d)
{
	return ((d*d)/4.)*(- 2*(x[0]*(x[0]-1.) + x[1]*(x[1]-1.)));
}

double calc_sphere_rad2D(double x[], double D[])
{
	double m0, m1, m2, m3;
	unsigned int A, B, C;
	unsigned int index;

	m0 = D[0] - x[0];
	m1 = x[0];
	m2 = D[1] - x[1];
	m3 = x[1];
#if 1
	A = (m1<m0);
	B = (m2<m0) & (m2<m1);
	C = (m3<m0) & (m3<m1) & (m3<m2);
	index = ((C|B)<<1) | ((C|(A&(!B)))&1);
#else
	A = (m1 < m0);
	B = (m3 < m2);
	C = (cur_min[B+2] < cur_min[A]);
	index = (C<<1) | ((B&C)|(A&(!C))&1);
#endif
	switch(index) {
		case 0: return m0;
		case 1: return m1;
		case 2: return m2;
		case 3: return m3;
	}
	assert(0);
}

void random_walk_2D(double x[], 
		double y[], double D[], double btol, double *est)
{
	double d;
	double t;

	t = 0;
	while ( (d = calc_sphere_rad2D(x, D)) > btol ) {
		update_y_2D(x, y, d);
		t += estimate_walk_2D(y, d);
		update_x_2D(x, d);
	}
	t += estimate_point_2D(x);
	*est = t;  
}

void random_walks_from_point(
		double D[],
		double x[], 
		double estimation[], 
		unsigned int num_walks, 
		double btol,
		unsigned int tasks)
{	
	double d;
	double est[tasks];
	double _x[2], _y[2];
	double factor = num_walks*tasks;
	unsigned int i, me;

	*estimation = 0;
	d = calc_sphere_rad2D(x, D);
	if ( d < 0 ) 
	{
		return;
	}
	else if ( d < btol )
	{
		*estimation = d;
		return;
	}

	for ( me=tasks; me > 0; --me) {
		est[me] = 0;
		// auto paei mesa se ena task
		for ( i=0; i<num_walks; ++i )
		{
			_x[0] = x[ 0 ];
			_x[1] = x[ 1 ];
			random_walk_2D(_x, _y, D, btol, &d);
			est[me] += d/(factor);
		}
	}

	d = est[0];
	for ( i=1; i<tasks; ++i )
	{
		d+=est[i];
	}
	*estimation = d;
}

int calc_nodes(double **nc, double D[], uint32_t nodes_X,
		uint32_t nodes_Y)
{
	uint32_t nof_nodes;
	uint32_t i, j, k;
	double *coords;
	const int block_size=12;

	nof_nodes = block_size * nodes_Y + block_size *nodes_X;
	coords = (double*) malloc(nof_nodes*2*sizeof(double));
	*nc = coords;
	k = 0;
	for ( i=0; i<block_size; ++i )
	{
		for ( j=0; j<nodes_Y; ++j )
		{
			coords[k*2] =(i+1)*(D[0]/(block_size + 1));
			coords[k*2+1] = j * D[1]/(nodes_Y-1);
			++k;
		}
	}
	for ( i=0; i<block_size; ++i )
	{
		for ( j=0; j<nodes_X; ++j )
		{
			coords[k*2] =(j)*(D[0]/(nodes_X - 1));
			coords[k*2+1] =(i+1)*(D[1]/(block_size + 1));
			++k;
		}
	}
	return nof_nodes;
}

long my_time()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec*1000000+tv.tv_usec;
}

int main(int argc, char* argv[])
{
	FILE *out;
	double D[2] = {1.0, 1.0}, btol = 1e-13;
	double *points, *estimation;
	int tasks;
	long nodes, dur;
	// parameters
	int nodes_x, nodes_y;
	int walks;
	int i;

	if ( argc != 6 )
	{
		printf("usage: %s ''N_X'' ''N_Y'' ''WALKS'' ''TASKS'' ''OUTPUT''\n", argv[0]);
		printf("Error");
		exit(1);
	}
	
	nodes_x = atoi(argv[1]);
	nodes_y = atoi(argv[2]);
	walks   = atoi(argv[3]);
	tasks   = atoi(argv[4]);
	out     = fopen(argv[5], "wb");
	// alloc points
	nodes = calc_nodes(&points, D, nodes_x, nodes_y);
	estimation = (double*) malloc(sizeof(double)*nodes);
	assert(points);
	assert(estimation);
	// execute for all points
	dur = my_time();

	for (i=0; i<nodes; ++i)
	{
		random_walks_from_point(D, points+2*i, estimation+i, walks, btol, tasks);
	}
	dur = my_time() - dur;
	printf("Duration: %ld\n", dur);
	fwrite(&nodes, sizeof(long), 1, out);
	fwrite(&dur, sizeof(long), 1, out);
	fwrite(estimation, sizeof(double), nodes, out);
	free(points);
	free(estimation);
	fclose(out);
	return 0;
}

