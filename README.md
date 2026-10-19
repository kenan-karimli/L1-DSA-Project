# L1 DSA Project

Final project for the UFAZ Data Structures & Algorithms course, written in C.

## Build & run

| Command      | What it does                                         |
|--------------|------------------------------------------------------|
| `make`       | build `build/app`                                    |
| `make run`   | build and run                                        |
| `make test`  | build and run the tests in `tests/`                  |
| `make debug` | build `build/debug/app` with AddressSanitizer (catches memory bugs), run tests |
| `make clean` | delete `build/`                                      |

## Layout

```
include/   headers: structs + function declarations only
src/       implementations; main.c only starts the UI
tests/     assert-based tests
```

New `.c` files in `src/` are picked up by the Makefile automatically.

## Rules

0. `main()` exists only in `src/main.c`. No other file in `src/` may define `main()`.
1. Code goes in `.c`, declarations in `.h` (start every header with `#pragma once`). Never `#include` a `.c` file.
2. Data-structure modules never call `printf`/`scanf`. They take arguments and return values or
   error codes. All input/output lives in `src/ui_*.c`, so the UI can be swapped later.
3. Every `malloc` has a matching `free`. Run `make debug` before merging.
4. `main` must always build. Work on `feature/<name>` branches; merge only after `make test` passes.
