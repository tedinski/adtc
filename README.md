
ADTC ~~is~~ _will be_ a code generator and library with three major goals:

1. **Make it convenient to develop highly memory-efficient data structures in C**.
C's support for data types ends at primitives, fixed-sized arrays, and structs.
Richer structures (trees especially) have to be constructed manually.
ADTC is meant to support doing so easily and efficiently.
2. **Support the use of these structures across the FFI into many other languages**.
Besides generating code to easily work with these data types directly within C, ADTC can also generate the code necessary code to use ADTC data from another language.
This allows libraries to communicate across FFI boundaries with richer data structures.
3. **Encourage message-passing between C threads.**
Instead of sharing mutable state, ADTC messages can be constructed and passed from one thread (or process) to another.
This approach isn't a panacea for writing concurrent programs, but it is highly effective and unnecessarily discouraged by the unfortunate difficulty of doing so in plain C.


The **distinguishing characteristic** of ADTC from similar tools (like Protocol Buffers, Flat Buffers, Cap'n Proto, etc) is that it targets **in-memory representation** rather than network protocols.
In particular, these other tools often target forward/backward compatibility, which is a non-goal for ADTC.
ADTC is perhaps most similar to Cap'n Proto, and an example of what ADTC does differently (better, for in-memory) can be found by looking at [Cap'n Proto's reasons for not having first-class tagged unions](https://capnproto.org/language.html#unions).


ADTC accomplishes these goals by several means:


* Memory management is simplified.
An ADTC buffer is managed normally for C, but all data allocated within it has the lifetime of the whole buffer.
This is common for many kinds of richer data: a JSON document, for instance, would correspond to an ADTC buffer, with all its constituent internal nodes tied to the lifetime of that buffer, needing no management of their own.
This is also the common case for transferring data across some boundary (FFI or thread, but disk or network, too): one side simply writes it, then the other side simply reads it.

* Representation is more efficient.
The flagship feature here (similar to Cap'n Proto) is changing internal pointers into indexes, reducing 64-bits to 32-bits.
Pointer-heavy data (trees, graphs) could be up to half the size.
ADTC does so with alignment in mind, meaning a single ADTC buffer can still hold 32 or 64+ GB of data typically.

* Common operations are automatically generated.
Similar to Haskell's `deriving` clause, many kinds of data structure traversals can be generated solely from the type.
Copies, comparisons, and generic traversals come for free.


## What does ADTC do?


ADTC is a library and code generator that takes a schema description of data types (in slightly extended version of otherwise ordinary C declaration syntax) and generates efficient low-level code and data structures in C to represent that data type.
These low-level representations are suitable for using from C, or via bindings to other high-level languages.
This allows C (and all supported higher-level languages) to easily represent rich data structures in a memory efficient, allocation efficient, and cache efficient manner.


This not only reduces memory use, but should improve performance, too.
See [`concept/`](concept/) for a proof-of-concept benchmark.


## What is an ADTC schema?


ADTC schemas are basic C type declarations, with a few modifications.
First, there's a few additions:

* ADTC has special "pointer types" (many are actually indexes).
* ADTC has special array types.
  * Java-style type-biased array syntax is permitted. e.g. `type[] var` instead of `type var[]`.
* ADTC has a special `String` type.
* ADTC has `tagged union` declarations (i.e. sum types).
  * Immediately within a tagged union, `void` becomes a valid type.
* ADTC supports limited notions of parameterized data types. (e.g. `List<T>`)
  * "Intrusive" (`List<T>`) must be manually instantiated for each desired type parameter.
  * "Incomplete" (`List<*T>`) will only generate one generic set of functions for the type.
  * This is to control the "code bloat" that comes from e.g. templates in C++.
* ADTC has a very basic "namespace" mechanism.


And there are a few modifications to the usual C semantics:

* C pointer syntax becomes ADTC "index pointers."
* C array syntax becomes ADTC arrays.
* `char *` and several common variants are automatically re-interpreted as ADTC `String`s.


The purpose of these changes is to make ordinary C types "automagically" become efficient representations within ADTC.
It also eases adoption because traditional C style becomes good ADTC style.


The special pointer types:

* `Index<a>` points to other data within the same ADTC buffer. This is what C-style pointers are re-interpreted as. They are not nullable (see `Maybe` below).
* `External<a>` is the pointer type for traditional (unmanaged) C pointers to arbitrary data elsewhere. External pointers may be null.
* `RemoteIndex<a>` is a special index type intended for an unspecified other buffer, not the local buffer.
* `Cursor<a>` points to other data within another ADTC buffer, essentially a combination of external pointer and remote index.
* `Maybe<a>` is not a pointer type, but is an ADTC type with special representation to make it efficient to use with pointer types. If you want a nullable index you'd use `Maybe<Index<a>>`. (`Maybe` is not necessary with external pointers, which can already be null.) There is added syntax to support `Maybe`: `Index<a>?` or `a *?`


The special array types:

* `Array<a>` is a standard ADTC variable-sized array, and is what C declarations like `int a[]` get translated to use (if the elements are not pointer types).
* `FixedArray<a>` is a standard ADTC fixed-size array, lacking size information (because it is static). C declarations like `struct foo a[4]` get translated to this.
* `DenseArray<a>` lays out the array as densely as possible, forsaking the "indexability" of its constituents. This is what arrays of pointers are automatically translated to.
* `DenseFixedArray<a>` lays out densely, and skips having size information. This is a traditional C array type, used with `native` and fixed size arrays of pointers.


With `Array` and `FixedArray` each element of the array will be aligned according to its containing ADTC buffer.
This means each element can be pointed to by an index pointer.
With `DenseArray` and `DenseFixedArray`, the elements are not indexable.
This means you can only access them by accessing a specific element of the array (or by native pointer).


## What does ADTC look like?


Some traditional types from functional programming languages, written in ADTC:

```
tagged union List<*Elem> {
  struct {
    Elem *head;
    List<Elem> *tail;
  } cons;
  void nil;
}

tagged union Either<*Left, *Right> {
  Left *left;
  Right *right;
}
```

This is all it take to represent JSON in ADTC:

```
tagged union JSON {
  void null;
  bool boole;
  double number;
  string string;
  JSON *array[];
  Pair<String, JSON*> object[]; // or a separate JSONObject type
}
```

"Intrusive" binary search trees, in ADTC:

```
tagged union BST<Elem> {
  Elem leaf; // Elem used as non-pointer, thus "intrusive"
  struct {
    BST<Elem> *left;
    BST<Elem> *right;
  } node;
}
```


## Can I just dump C declarations into ADTC and benefit?


Almost, but usually there are some considerations.
First, an ADTC structure is intended to be "one independent blob of data" so highly inter-connected data structures (where there are pointers in from elsewhere) aren't necessarily an immediate fit.
Second, some syntax is slightly more restrictive. (`long const long` is valid C, but ADTC requires `const long long`.
In general the order is `static inline const void __attribute__`. This is conventional, so you'll rarely run afoul of it.


A common pattern in C is to represent an array with a pointer, but this is incorrect in ADTC.
So you may have to find `type *var` and convert it to `type[] var` to get the ADTC semantics right.


Additionally, sometimes internal pointers are meant to be actually nullable, and with ADTC you have to indicate that by wrapping with `Maybe` e.g. by adding `?` such as `struct obj *? var`.


ADTC also allows `native struct` declarations that revert to default behaviors for handling arrays and pointers and such, to be perfectly compatible with C.
Imported types from plain C headers are treated as such automatically.


## What else does ADTC do?


ADTC schemas are not just the declarations of types, but also contain:

* Code generation directives.
* Linting directives. (Checking security properties, size, waste, etc.)
* Instantiations of parameterized ADTC types at certain concrete types (essential for "intrusive" types).
* Inline functions.
* Function prototypes.
* Simple recursive functions.


By convention, an ADTC file is meant to replace a header file.
You would have `mytype.adtc` which would generate `mytype.h` and `mytype.adtc.c`, and you would use `mytype.h` normally from your other C code, with custom plain C functions (that you have prototypes for in `mytype.adtc`) appearing in `mytype.c`.
The code generated by ADTC should be extremely readable, the header especially.


In other words, ADTC acts like an "enhanced header file" that also generates some nice implementations for you.
As such, it should be used with the same conventions as a normal header file.


Generated code can include:

* Type-specialized code
* Allocation functions
* Copy functions
* Deallocation / generic traversal functions
* Accessor functions
* Allocation buffer helper structures and functions


## Even Future-er work


ADTC presently has no "story" for evolution of data schemas over time.
(We primarily target in-memory representations, so that's somewhat irrelevant.)
While it is a **non-goal** to support using ADTC to describe protocols (use protocol buffers or something designed for that), it could still be useful to address loading "old" data to a limited degree.
(For example, so that programs written against old versions of a library can still work with new versions.)
One possible goal would be to clear about what changes can be made to ADTC schemas and still be able to load old data.
(We might also offer "versioned" structs, which also encode their size, allowing them to grow over time, while new code can recognize old structs and thus not access non-existant fields.)


Integration with external tools.
We'd like to parse protocol buffers and other schema formats, and generate ADTC schemas from them.
Additionally, it'd be nice to generate integrations with appropriate parsing code (so you can parse protocol buffer data directly into an ADTC representation.)
Likewise with an efficient JSON parser.


ADTC should likely come with a small standard library.
A built-in `Dict` type is a distinct possibility.


ADTC is presently a (not even a) **prototype** implementation.
It is subject to breaking changes.
And the ADTC compiler is implemented in an academic research language, not plain C (yet).
All that will be fixed for a 1.0 release.


## Repo structure (planned)


```
concept/            (independent demo of performance benchmarks)
include/adtc.adtc   (header file for adtc base types and standard library of adtc)
libadtc/adtc.c      (the custom code supporting adtc.h which is generated by adtc.adtc)
prototype/          (the prototype adtc compiler implementation, to be replaced)
adtcc/              (the home of the future production compiler implementation)
test/               (test suite)
ffi/python/         (adtc compiler plugin to generate python FFI bindings)
ffi/java/           (adtc compiler plugin to generate java FFI bindings)
```

