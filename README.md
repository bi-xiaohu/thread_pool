# mycpp-template-vcpkg
A modern, cross-platform C++ project template with CMake, vcpkg, CI, tests, benchmarks and sanitizers.

## Directory Layout

```text
.
├── app/        # Executables
├── bench/      # Benchmarks
├── cmake/      # CMake modules
├── include/    # Public headers
├── src/        # Core library source
├── tests/      # Unit tests
├── tools/      # Dev tools / scripts
└── .github/
    └── workflows/  # CI
```

## BUILD

```bash
cmake -S . -B build
cmake --build build
```
