# bitmap

A C++ implementation of a bitmap (bitset) data structure.

The full development plan lives in ROADMAP.md, which includes
C++ reference material, bitmap theory, and a step-by-step
build plan.

## Status

Work in progress.

Done:
- include/bitmap/bitmap.hpp - interface
- src/bitmap.cpp - implementation

Not yet:
- src/bitmap_test.cpp - test program (CMake target, not yet written)
- full test suite

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
    bm_check

## Requirements

- A C++17 compiler (g++ or clang++)
- CMake 3.20 or newer

## Build

    mkdir -p build && cd build
    cmake ..
    cmake --build .

## Test

    ./build/bitmap_test

(Test program not yet written. Build will fail at link
until src/bitmap_test.cpp exists.)

## Layout

    include/bitmap/    public headers
    src/               library and test sources
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
