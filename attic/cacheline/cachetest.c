#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

// example
//gcc -O4 -DCACHELOOP=16 cachetest.c -lpthread; a.out 1 10000 16000

// usage: a.out nthread niter ng
// nvar refers to number of triples where a = (b - a)*c
// ng refers to total number of gs/clsize groups where
// gs is size of array of neqn triples separated by varsep where
// each item is clsize doubles. ie. total memory is
// nthread * ng * neqn*3*varsep*clsize

int nthread, niter, ng, neqn, varsep, clsize;
int gs, aoff, boff, coff, asep;

static double start_time = 0.;
double nrn_time() {
	int ms10;
	struct timeval x;
	gettimeofday(&x, (struct timezone*)0);
	ms10 = x.tv_usec/10000;
	start_time = (100.*(double)(x.tv_sec) + (double)ms10)/100.;
	return (start_time);
}

#if CACHELOOP
#define cldec int clj;
#define cls CACHELOOP
#define cloff * cls + clj
#define clb for (clj=0; clj < cls; ++clj) {
#define cle }
#else
#define cldec /**/
#define cloff /**/
#define cls 1
#define clj 0
#define clb /**/
#define cle /**/
#endif

#define a1 p[0 cloff]
#define b1 p[5 cloff]
#define c1 p[10 cloff]
#define a2 p[15 cloff]
#define b2 p[20 cloff]
#define c2 p[25 cloff]
#define a3 p[30 cloff]
#define b3 p[35 cloff]
#define c3 p[40 cloff]

static double* val[32];

static void iter(double* p) {
	cldec
	int ig;
	for (ig=0; ig < ng; ++ig) {
		clb a1 = 0.; a2 = 0.; a3 = 0.; cle
		clb b1 = 1.; b2 = 1.; b3 = 1.; cle
		clb c1 = .001; c2 = .002; c3 = .003; cle
	}
}

static void* waste(void* v) {
	cldec
	int iiter, ig, ieqn, ith, icl;
	int ia, ib, ic;
	double* vt, *p;
	int a, b, c;
	ith = (int)((long)v);
	vt = val[ith];
	iter(vt);
	for (iiter=0; iiter < niter; ++iiter) {
		p = vt;
		for (ig=0; ig < ng; ++ig) {
			clb a1 = a1 + (b1 - a1)*c1; cle
			clb a2 = a2 + (b2 - a2)*c2; cle
			clb a3 = a3 + (b3 - a3)*c3; cle
			p = vt + gs;
		}
	}
	return (void*)0;
}

#if 0 /* do not need -lpthread */
static double trial(int nth) {
	long i;
	double t;
	t = nrn_time();
	waste(0);
	t = nrn_time() - t;
	return t;
}
#else
static double trial(int nth) {
	long i;
	double t;
	pthread_t* th;
	th = (pthread_t*)calloc(nth, sizeof(pthread_t));
	t = nrn_time();
	for (i=0; i < nth; ++i) {
		pthread_create(th + i, (void*)0, waste, (void*)i);
	}
	for (i=0; i < nth; ++i) {
		pthread_join(th[i], (void*)0);
	}
	t = nrn_time() - t;
	free((char*)th);
	return t;
}
#endif

int main(int argc, char** argv) {
	int i;
	// usage: a.out nthread niter ng varsep neqn clsize
	sscanf(argv[1], "%d", &nthread);
	sscanf(argv[2], "%d", &niter);
	sscanf(argv[3], "%d", &ng);
	varsep = 5;
	neqn = 3;
	clsize = cls;
	ng = ng/clsize;
	gs = neqn*3*varsep*clsize;
	printf("niter=%d nthread=%d ng=%d varsep=%d neqn=%d clsize=%d data/thread=%d\n",
		niter, nthread, ng, varsep, neqn, clsize, gs*ng);
//	printf("pagesize=%d\n", getpagesize());
	for (i=0;i < nthread; ++i) {
		val[i] = (double*)calloc(ng*gs, sizeof(double));
	}
	printf("t=%g\n", trial(nthread));
	return 0;
}
