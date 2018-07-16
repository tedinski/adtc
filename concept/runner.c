#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct timespec timestamp;

static inline timestamp
timestamp_now()
{
	timestamp t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t;
}

static inline double
timestamp_difference(timestamp earlier, timestamp later)
{
	return
		(later.tv_sec - earlier.tv_sec) +
		(later.tv_nsec - earlier.tv_nsec) * 1.0E-9;
}

// API for different implementations
#include "tree.h"

int get_memory_usage_kb(long* vmrss_kb, long* vmsize_kb);

int main(int argc, char *argv[])
{
	int bench_depth;
	if(argc == 1) {
		bench_depth = 26;
	} else if(argc == 2) {
		bench_depth = atoi(argv[1]);
	} else {
		abort();
	}
		
	long vmrss, vmsize;
	
	printf("Generate \t Copy   \t Delete \t Total  \t VM \n");

	for(int i = 0; i < 5; i++) {

		struct tree_root *tree = tree_new();
	
		timestamp start = timestamp_now();
	
		generate_into(tree, bench_depth);
	
		timestamp middle = timestamp_now();
	
		struct tree_root *tree2 = tree_copy(tree);
	
		timestamp late = timestamp_now();
	
		get_memory_usage_kb(&vmrss, &vmsize);
	
		tree_delete(tree);
		tree_delete(tree2);
	
		timestamp end = timestamp_now();
	
		printf("%lf \t %lf \t %lf \t %lf \t %ld MB \n",
			timestamp_difference(start, middle),
			timestamp_difference(middle, late),
			timestamp_difference(late, end),
			timestamp_difference(start, end),
			vmsize / 1024);
	}
}
