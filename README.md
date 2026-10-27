# bitmap

A C++ implementation of a bitmap (bitset) data structure.

The full development plan lives in ROADMAP.md, which includes
C++ reference material, bitmap theory, and a step-by-step
build plan.

## Status

Done:
- include/bitmap/bitmap.hpp - interface
- src/bitmap.cpp - implementation
- src/bitmap_test.cpp - full test suite (12 sections)
- src/bitmap_demo.cpp - CP+ disk block allocator demo

## API summary

Lifecycle:
    bm_init, bm_free, bm_valid

Basic:
    bm_set, bm_clear, bm_test, bm_toggle
    bm_size_of, bm_count

Mass:
    bm_set_all, bm_clear_all, bm_print

Search:
    bm_find_first_set, bm_find_first_clear, bm_recount

Logical:
    bm_and, bm_or, bm_xor, bm_not, bm_copy

Diagnostics:
    bm_check, bm_error_string

## Requirements

- A C++17 compiler (g++ or clang++)
- CMake 3.20 or newer

## Build

    mkdir -p build && cd build
    cmake ..
    cmake --build .

## Test

    ./build/bitmap_test
    ./build/bitmap_demo

The test suite covers 12 sections: empty bitmap, single bit,
word boundaries, basic ops, mass ops, find, recount, logic,
NOT with trailing bits, check diagnostics, error strings, and
invalid calls. The demo emulates a CP+ disk block allocator on
top of the bitmap and exercises the library end to end.

## Layout

    include/bitmap/    public headers
    src/               library, test, and demo sources
    ROADMAP.md         development plan and reference
    CMakeLists.txt     build configuration
    README.md          this document
    LICENSE            MIT license text

## Design notes

- 80s C-flavoured C++.
- Fixed-size Bitmap object, free functions bm_*.
- Error codes via #define BM_ERR_*, no exceptions.
- Explicit types, explicit prefixes.
- Invariant: trailing bits in the last word are always 0.

## License

MIT, Copyright (c) 2026 Albert Ghosting Frangle (Albert Fisher)
