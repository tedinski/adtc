CFLAGS=-Wall -g -O3 -fno-omit-frame-pointer -DNDEBUG
FG := ~/repos/FlameGraph

all: run_malloc run_buffer

run_malloc: runner.o tree_malloc.o vmsize.o
	gcc -o $@ $^

run_buffer: runner.o tree_buffers.o vmsize.o buffers.o
	gcc -o $@ $^

clean:
	rm -f *.o runner run_malloc run_buffer perf-out-* perf-*.svg

bench: all
	./run_malloc
	./run_buffer

profile: all
	perf record -F 999 -o perf-out-malloc -g -- ./run_malloc
	perf record -F 999 -o perf-out-buffer -g -- ./run_buffer
	perf script -i perf-out-malloc | $(FG)/stackcollapse-perf.pl | $(FG)/flamegraph.pl > perf-malloc.svg
	perf script -i perf-out-buffer | $(FG)/stackcollapse-perf.pl | $(FG)/flamegraph.pl > perf-buffer.svg

valgrind: all
	valgrind ./run_malloc 10
	valgrind ./run_buffer 10
