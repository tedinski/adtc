
ADTC is a tool for generating representations of data in C.
The goals of ADTC include:

* Encourage a style of programming that makes greater emphasis on passing around rich data between components.
* Make it more feasible to represent tree-like data in C. ("Algebraic Data Types for C" was the original working name, but then we just decided ADTC had a ring to it.)
* Enable extremely efficient representations of data.
* Target in-memory representations, rather than protocols.
* Attempt to control ownership issues by going with a "document as data" design.
* Generate as much code as possible.

## My personal TODO List

This repo is experimental, probably shouldn't be using it yet.

Ideas to think on:

* cross platform annotations (e.g. packed)
* simple functions
* embedded values (stuff into `.rodata`)
* destruction functions
* mini-heap garbage collection

## What does ADTC do?

ADTC is a library and code generator that takes a high-level schema description of data and generates efficient low-level data structures in C to represent that data.
These low-level representations are suitable for using from C, or via bindings to other high-level languages.
This allows C (and all supported higher-level languages) to easily represent rich data structures in a memory efficient, allocation efficient, and cache efficient (enabling runtime improvements, too) manner.

## What is an ADTC schema?

ADTC schemas are basic C type declarations, with a few modifications.
First, there's a few additions:

* ADTC has special "pointer types."
* ADTC has special array types.
* ADTC has a special `String` type.
* ADTC has `tagged union` declarations (i.e. sum types).
  * Immediately within a tagged union, `void` becomes a valid type.
* ADTC supports limited notions of parameterized data types. (e.g. `List<a>`)
* ADTC has a primitive "namespace" mechanism.

And there are a few modifications to the usual C semantics:

* C pointer syntax becomes ADTC "index pointers."
* C array syntax becomes ADTC arrays.
* `char *` and several common variants are re-interpreted as ADTC `String`s.
* Java-style type-biased array is permitted e.g. `type[] var` instead of `type var[]`.

The purpose of these changes is to make C types "automagically" become efficient representations within ADTC.
It also eases adoption by encouraging ADTC style (because traditional C style becomes good ADTC style.)

The special pointer types:

* `Index<a>` points to other data within the same ADTC buffer. This is what C-style pointers are re-interpreted as. They are not nullable (see `Maybe` below).
* `External<a>` is the pointer type for traditional (unmanaged) C pointers to arbitrary data elsewhere.
* `Cursor<a>` points to other data within another ADTC buffer, essentially a combination of external pointer and index.
* `Maybe<a>` is not a pointer type, but is an ADTC type with special representation to make it efficient to use with pointer types. If you want a nullable index you'd use `Maybe<Index<a>>`. (`Maybe` is not necessary with external pointers, which can already be null.) There is added syntax to support `Maybe`: `Index<a?>` or `a *?`

The special array types:

* `Array<a>` as a standard ADTC variable size array, and is what C types like `struct foo a[]` get translated to (if the elements are not pointer types).
* `FixedArray<a>` is a standard ADTC fixed-size array, lacking size information (because it is static). C types like `struct foo a[4]` get translated to this.
* `DenseArray<a>` lays out the array as densely as possible, forsaking the "indexability" of its constituents. This is what arrays of pointers are automatically translated to.
* `DenseFixedArray<a>` lays out densely, and skips having size information. This is a traditional C array type, used with `native` and fixed size arrays of pointers.

## What does ADTC look like?

Some traditional types from functional programming languages, written in ADTC:

```
tagged union List<Elem> {
  struct {
    Elem *head;
    List<Elem> *tail;
  } cons;
  void nil;
}

tagged union Either<Left, Right> {
  Left *left;
  Right *right;
}
```

JSON, written in ADTC:

```
tagged union JSON {
  void null;
  bool bool;
  double number;
  string string;
  JSON array[];
  Pair<String, JSON> object[];
}
```

"Intrusive" binary search trees, in ADTC:

```
tagged union BST<Elem> {
  Elem leaf; // Non-pointer, thus "intrusive"
  struct {
    BST<Elem> *left;
    BST<Elem> *right;
  } node;
}
```

## Can I just dump C declarations into ADTC?

Almost, but usually there are some considerations.

A common pattern in C is to represent an array with a pointer.
So you may have to find `type *var` and convert it to `type[] var` to get the ADTC semantics right.

Additionally, sometimes pointers are meant to be actually nullable, and with ADTC you have to indicate that by wrapping with `Maybe` e.g. `Maybe<struct obj *> var`.

ADTC also allows `native struct` declarations that revert to default behaviors for handling arrays and pointers and such, to be perfectly compatible with C.
Imported types from headers are treated as such automatically.

## What else does ADTC do?

Schemas are not just the declarations of types, but also:

* Code generation directives.
* Linting directives.
* Instantiations of parameterized ADTC types at certain concrete types.

* Inline functions.
* Function prototypes.
* Simple recursive functions.

By convention, an ADTC file is meant to replace a header file.
You would have `mytype.adtc` which would generate `mytype.h` and `mytype.adtc.c`, and you would use `mytype.h` normally from your other C code, with custom plain C functions (that you have prototypes for in `mytype.adtc`) appearing in `mytype.c`
The code generated by ADTC should be extremely readable, the header especially.

In other words, ADTC acts like an "enhanced header file" that also generates some nice implementations for you.
As such, it should be used like a normal header file.

Generated code can include:

* Allocation functions
* Copy functions
* Deallocation functions
* Accessor functions
* Allocation buffer helper structures and functions

## Future work

ADTC presently has no "story" for evolution of data schemas over time.
(We primarily target in-memory representations, so that's irrelevant.)
While it is a **non-goal** to support using ADTC to describe protocols (use protocol buffers or something designed for that), it could still be useful to address loading "old" data to a limited degree.
One possible goal would be to clear about what changes can be made to ADTC schemas and still be able to load old data.
(We might also offer "versioned" structs, which also encode their size, allowing them to grow over time, while new code can recognize old structs and thus not access off the end.)

Integration with external tools.
We'd like to parse protocol buffers and other schema formats, and generate ADTC schemas from them.
Additionally, it'd be nice to generate integrations with appropriate parsing code (so you can parse protocol buffer data directly into an ADTC representation.)
Likewise with an efficient JSON parser.

ADTC is presently a **prototype** implementation.
It is subject to breaking changes.
And the ADTC compiler is implemented in an academic research language, not C (yet).
We'll fix that for 1.0.


## File structure (planned)

```
include/adtc.adtc   (header file for adtc base types and STDLIB of adtc)
libadtc/adtc.c      (the custom code supporting adtc.h which is generated by adtc.adtc)
prototype/          (the prototype adtc compiler implementation, to be replaced)
adtc/               (the home of the future production implementation)
test/               (test suite)
ffi/python/         (adtc compiler plugin to generate python FFI bindings)
ffi/java/           (adtc compiler plugin to generate java FFI bindings)
```

