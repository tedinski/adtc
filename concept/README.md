This is a quick demonstration of the performance ideas underlying ADTC.
This benchmark does something quite simple: it compares the performance of two approaches to creating a tree, copying the tree, and deleting both trees.

1. `malloc`: the tree is allocated in the usual fashion, each node getting allocated independently.
2. `buffer`: the tree is represented in an ADTC-like fashion, the whole tree allocated inside a single buffer.

In general, the ideas on display here are (1) using indexes instead of pointers makes for denser data structures, and (2) using a single buffer instead of large numbers of allocations helps reduce runtime overhead.

## Results

`./run_malloc`

| Generate    | Copy      | Delete    | Total     | VM
|----|-|-|-|-
| 2.355     | 2.292     | 2.257     | 6.904     | 4100 MB
| 1.582     | 1.884     | 2.989     | 6.455     | 4100 MB
| 1.168     | 1.111     | 2.250     | 4.529     | 4100 MB
| 1.581     | 1.873     | 3.007     | 6.460     | 4100 MB
| 1.177     | 1.109     | 2.273     | 4.559     | 4100 MB

`./run_buffer`

| Generate    | Copy      | Delete    | Total     | VM
|----|-|-|-|-
| 1.052     | 1.150     | 0.046     | 2.248     | 2276 MB
| 1.073     | 1.158     | 0.046     | 2.277     | 2276 MB
| 1.048     | 1.147     | 0.047     | 2.241     | 2276 MB
| 1.074     | 1.149     | 0.045     | 2.269     | 2276 MB
| 1.063     | 1.175     | 0.050     | 2.288     | 2276 MB

### Summary

Terminology Note: that we use "% faster" and "% less memory" to mean that the original memory usage is reduced by this percentage. E.g. If we started with 100 MB of memory, using 30% less memory means using 70 MB. Or a runtime of 100s becomes 70s. I.e. `1 - (new / orig)`.

| | Malloc-style | Buffer-style | Buffer %-better
|---|-|-|-
| Generate time | 1.57 s | 1.06 s | 32%
| Copy time | 1.65 s | 1.16 s | 30%
| Delete time | 2.55 s | 0.05 s | 98%
| Total time | 5.78 s | 2.26 s | 61%
| Standard deviation | 1.14 s | 0.02 s | 98%
| Memory usage | 4100 MB | 2276 MB | 44%

So depending on how you want to measure things:

* The buffer approach is 30-60% faster. (Or to use different terminology, _twice the speed_.)
* The buffer approach uses 44% less memory.
* The buffer approach is vastly more consistent in run times (or latencies).

## Discussion

**Isn't the lack of deletion time for buffers cheating?**

No, this is part of the intended test: lack of need to traverse all data to deallocate it is a legitimate advantage of the buffer-based approach.
We wanted a benchmark that reflected that advantage at least somewhat.
You could argue this one reflects it too much, but it is a real performance advantage.
But nevertheless: note the 30% straight runtime performance advantage in generate and copy times.

**Why the larger standard deviation in the malloc strategy?**

Not totally certain.
There appears to be two effects: first run, and then alternating runs.
We believe the first run slow down is related to the process actually allocating space from the operating system.
After that first run, it looks like the process is holding onto and re-using it.

We believe the alternating run slow downs are a result of some amortized process withing glibc's malloc implementation.
That is, some garbage-collection-like occasional latencies inherent to allocating and freeing a lot.

Regardless, it seems to also be a significant advantage for the buffer approach: much more consistent run times, for doing the same work.

**Doesn't this benchmark disadvantage (insert approach here)?**

Somewhat, for both approaches.
The test harness was written to be as uniform as possible between the two approaches, and this introduces overheads _for both approaches_.
These are related to passing around unnecessary arguments, or recalculating some pointers.
We think it's a wash.


