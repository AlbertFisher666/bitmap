# bitmap

A C++ implementation of a bitmap (bitset) data structure.

The full development plan lives in ROADMAP.md, which includes
C++ reference material, bitmap theory, and a step-by-step
build plan.

## Status

Work in progress. See ROADMAP.md for the current phase.

## Requirements

- A C++17 compiler (g++ or clang++)
- CMake 3.20 or newer

## Build

    mkdir -p build && cd build
    cmake ..
    cmake --build .

## Test

    ./build/bitmap_test

## Layout

    include/bitmap/    public headers
    src/               library and test sources
    ROADMAP.md         development plan and reference
    CMakeLists.txt     build configuration
    README.md          this document
    LICENSE            MIT license text

## License

MIT, Copyright (c) 2026 Albert Ghosting Frangle (Albert Fisher)
