
This is a quick demonstration of the performance ideas underlying ADTC.
This benchmark does something quite simple: it compares the performance of two approaches to creating a tree, copying the tree, and then deleting both trees.
These approaches are:

1. `malloc`: the tree is allocated in the usual fashion, each node getting allocated independently.
2. `buffer`: the tree is represented in an ADTC-like fashion, the whole tree allocated inside a single buffer.


In general, the ideas on display here are (1) using indexes instead of pointers makes for denser data structures, and (2) using a single buffer instead of large numbers of allocations helps reduce runtime overhead.


## Results


Here are the runtime results for the two different benchmarking runs:


`./run_malloc`

| Generate    | Copy      | Delete    | Total     | VM
|----|-|-|-|-
| 2.355 s    | 2.292 s    | 2.257 s    | 6.904 s    | 4100 MB
| 1.582 s    | 1.884 s    | 2.989 s    | 6.455 s    | 4100 MB
| 1.168 s    | 1.111 s    | 2.250 s    | 4.529 s    | 4100 MB
| 1.581 s    | 1.873 s    | 3.007 s    | 6.460 s    | 4100 MB
| 1.177 s    | 1.109 s    | 2.273 s    | 4.559 s    | 4100 MB


`./run_buffer`

| Generate    | Copy      | Delete    | Total     | VM
|----|-|-|-|-
| 1.052 s    | 1.150 s    | 0.046 s    | 2.248 s    | 2276 MB
| 1.073 s    | 1.158 s    | 0.046 s    | 2.277 s    | 2276 MB
| 1.048 s    | 1.147 s    | 0.047 s    | 2.241 s    | 2276 MB
| 1.074 s    | 1.149 s    | 0.045 s    | 2.269 s    | 2276 MB
| 1.063 s    | 1.175 s    | 0.050 s    | 2.288 s    | 2276 MB


### Summary


Terminology Note: that we use "% faster" and "% less memory" to mean that the original memory usage is reduced by this percentage. E.g. If we started with 100 MB of memory, using 30% less memory means using 70 MB. A runtime that's 50% faster would go from 100s to 50s. I.e. `1 - (new / orig)`.


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


**Isn't the lack of deletion time for ADTC buffers cheating?**

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


