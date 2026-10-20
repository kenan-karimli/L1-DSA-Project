# L1 DSA Project

Final project for the UFAZ Data Structures & Algorithms course, written in C.

## Build & run

| Command      | What it does                                         |
|--------------|------------------------------------------------------|
| `make`       | build `build/app`                                    |
| `make run`   | build and run                                        |
| `make test`  | build and run the tests in `tests/`                  |
| `make debug` | build `build/debug/app` with AddressSanitizer (catches memory bugs), run tests |
| `make submit` | merge modules into `submit/tp3.h`, `submit/tp3.c`, `submit/main.c` and check they compile |
| `make lib`   | build `build/libhotel.so` (the C code as a shared library, for the GUI) |
| `make gui`   | build the library and open the tkinter GUI (`make gui PYTHON=<python with tkinter>`) |
| `make clean` | delete `build/` and `submit/`                        |

## Layout

```
include/   headers: structs + function declarations only
src/       implementations; main.c is the terminal menu
tests/     assert-based tests
data/      bookings.txt, the sample file read by processingfile
gui/       hotel_gui.py, a tkinter GUI that calls the same C functions through ctypes
```

New `.c` files in `src/` are picked up by the Makefile automatically.

## Rules

0. `main()` exists only in `src/main.c`. No other file in `src/` may define `main()`.
1. Code goes in `.c`, declarations in `.h` (start every header with `#pragma once`). Never `#include` a `.c` file.
2. Data-structure modules never call `scanf` and only print where the statement requires it
   (`displayHotel`, `displaySite`, `searchClients`, `processingfile`). They take arguments and return
   values or error codes. The terminal menu lives in `src/main.c`; the GUI uses `findClients` and walks the lists itself.
3. Every `malloc` has a matching `free`. Run `make debug` before merging.
4. `main` must always build. Work on `feature/<name>` branches; merge only after `make test` passes.
