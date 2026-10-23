# L1 DSA Project - Hotel chain

Our final project for the Data Structures & Algorithms course at UFAZ. It's a hotel chain manager
written in C with singly linked lists (chain -> locations -> customers).

## How to build / run

| Command       | What it does |
|---------------|--------------|
| `make`        | compiles everything into `build/app` |
| `make run`    | compiles and runs it |
| `make test`   | runs our tests from `tests/` |
| `make debug`  | same but with AddressSanitizer, shows memory leaks and bad accesses |
| `make submit` | puts all our files together into `submit/tp3.h`, `submit/tp3.c`, `submit/main.c` (the 3 files we have to upload) and checks they compile |
| `make lib`    | builds `build/libhotel.so` so python can use our C code |
| `make gui`    | opens the tkinter GUI (`make gui PYTHON=<python with tkinter>` if your python doesn't have it) |
| `make web`    | web version, open http://127.0.0.1:8000 |
| `make clean`  | deletes `build/` and `submit/` |

## Folders

```
include/   .h files (structs and function prototypes)
src/       .c files, main.c has the menu
tests/     tests with assert
data/      bookings.txt (the example file for processingfile)
gui/       hotel_lib.py loads the C library with ctypes,
           hotel_gui.py is the tkinter version, hotel_web.py + hotel_web.html is the web version
```

If you add a new `.c` file in `src/` the Makefile finds it by itself.

## Saving

Option 11 in the menu (or **Save** in the GUI / web page) saves everything to `data/saved_chain.txt`
with `saveHotel`. It's the same format as `data/bookings.txt`. Option 12 (**Load saved**) loads it
back with `loadBookings`. The chain name and empty locations are not saved.

## Our rules

0. `main()` is only in `src/main.c`, nowhere else.
1. Code in `.c`, prototypes in `.h` (every header starts with `#pragma once`). Never `#include` a `.c` file.
2. No `scanf` in the list functions. They only print when the subject says so
   (`displayHotel`, `displaySite`, `searchClients`, `processingfile`), otherwise they just return something.
   All the menu stuff is in `src/main.c`. The GUI uses `findClients` and goes through the lists itself.
3. Every `malloc` needs a `free`. Run `make debug` before merging.
4. `main` branch must always compile. Work on `feature/<name>` and merge only if `make test` passes.
