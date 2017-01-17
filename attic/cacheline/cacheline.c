#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

static double start_time = 0.;
double nrn_time() {
	int ms10;
	struct timeval x;
	gettimeofday(&x, (struct timezone*)0);
	ms10 = x.tv_usec/10000;
	start_time = (100.*(double)(x.tv_sec) + (double)ms10)/100.;
	return (start_time);
}

static int* val[32];

static void* waste(void* v) {
	long i, j, n;
	j = (long)v;
	n = 100000000;
	*val[j] = 0;
	for (i=0; i < n; ++i) {
		*val[j] += i;
	}
	return (void*)0;
}

static double trial(int ip) {
	long i;
	double t;
	pthread_t* th;
	th = (pthread_t*)calloc(ip, sizeof(pthread_t));
	t = nrn_time();
	for (i=0; i < ip; ++i) {
		pthread_create(th + i, (void*)0, waste, (void*)i);
	}
	for (i=0; i < ip; ++i) {
		pthread_join(th[i], (void*)0);
	}
	t = nrn_time() - t;
	free((char*)th);
	return t;
}

int main(int argc, char** argv) {
	int i, n, stride;
	sscanf(argv[1], "%d", &n);
	sscanf(argv[2], "%d", &stride);
	printf("n=%d stride=%d sizeof(int)=%d sizeof(long)=%d sizeof(pthread_t)=%d\n",
		n, stride, sizeof(int), sizeof(long), sizeof(pthread_t));
	printf("pagesize=%d\n", getpagesize());
	val[0] = (int*)malloc(n*stride*sizeof(int));
	for (i=1;i < n; ++i) {
		val[i] = val[i-1] + stride;
	}
	for (i=1; i < n; ++i) {
		char* c1 = (char*)val[i];
		char* c2 = (char*)val[i-1];
		printf("val[%d]-val[%d] = %d (bytes)\n", i, i-1, c1 - c2);
	}
	printf("t=%g\n", trial(n));
	return 0;
}
