# bitmap

A C++ implementation of a bitmap (bitset) data structure.

The full development plan lives in ROADMAP.md, which includes
C++ reference material, bitmap theory, and a step-by-step
build plan.

## Status

Done:
- include/bitmap/bitmap.hpp - interface
- src/bitmap.cpp - implementation
- src/bitmap_test.cpp - full test suite (13 sections)
- src/bitmap_demo.cpp - CP+ disk block allocator demo
- ASan/UBSan clean (no leaks, no UB)

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

The test suite covers 13 sections: empty bitmap, single bit,
word boundaries, basic ops, mass ops, find, recount, logic,
NOT with trailing bits, check diagnostics, error strings,
double init, and invalid calls. The demo emulates a CP+ disk
block allocator on top of the bitmap and exercises the
library end to end.

Sanitizers (built into GCC/Clang, no install required):

    g++ -std=c++17 -g -O1 -fsanitize=address,undefined \
        -fno-omit-frame-pointer \
        -Iinclude src/bitmap.cpp src/bitmap_test.cpp \
        -o /tmp/bitmap_test_asan
    /tmp/bitmap_test_asan

    g++ -std=c++17 -g -O1 -fsanitize=address,undefined \
        -fno-omit-frame-pointer \
        -Iinclude src/bitmap.cpp src/bitmap_demo.cpp \
        -o /tmp/bitmap_demo_asan
    /tmp/bitmap_demo_asan

ASan should produce no output. If it does, it points at the
file and line of the leak or UB.

## Layout

    include/bitmap/    public headers
    src/               library, test, and demo sources
    build/             build output (gitignored)
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
- NULL semantics: bm_init returns BM_ERR_INVALID, bm_free is
  a no-op, bm_size_of/bm_count return 0, bm_check returns 1.
- bm_init frees previous storage before re-init.
- bm_test returns 0 both for "clear" and for "error"; callers
  that need to distinguish must check bm_valid / index.

## License

MIT, Copyright (c) 2026 Albert Ghosting Frangle (Albert Fisher)
