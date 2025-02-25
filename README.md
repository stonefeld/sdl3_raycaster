# SDL3 Raycaster Engine

A POC of a raycaster engine using SDL3

## Requirements

* `cmake`
* Any C compiler like `gcc` or `MSVC`

## Building

```bash
# On Linux/MacOS
cmake -S . -B build
cmake --build build --target raycaster

# On Windows
cmake -S . -B build                       # tries to use MSVC if installed
cmake -S . -B build -G "MinGW Makefiles"  # MinGW has to be installed
cmake --build build --target raycaster
```

All executable files and assets will be located in the `build` directory
