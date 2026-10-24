# L1 DSA Project - Hotel chain

Our final project for the Data Structures & Algorithms course at UFAZ. A hotel chain manager
written in C with singly linked lists (chain -> locations -> customers).

## Folders

```
submit/   what we upload on Moodle: tp3.h, tp3.c, main.c (+ the report)
tests/    test_tp3.c, tests for submit/tp3.c with assert
data/     bookings.txt, the example file read by processingfile
web/      extras, NOT submitted: web app + tkinter GUI (Python) and web.c/web.h,
          the C functions only the GUIs need (loadBookings, saveHotel, saveChainFile)
docs/     the project statement PDF
```

## Commands

| Command      | What it does |
|--------------|--------------|
| `make run`   | compiles `submit/` and runs the menu |
| `make test`  | runs `tests/test_tp3.c` and `web/test_web.c` |
| `make debug` | same tests with AddressSanitizer (memory leaks, bad accesses) |
| `make web`   | web version, open http://127.0.0.1:8000 |
| `make gui`   | tkinter version (`make gui PYTHON=<python with tkinter>`) |
| `make clean` | deletes `build/` |

Run everything from the project folder, so `data/bookings.txt` is found.

## Web version

Accounts (login + password) get a "My chains" list, saved to `data/chains/<login>/`.
Anyone can also open a `.txt` chain file; files can have an optional file password.
Passwords are never saved, only salted pbkdf2 hashes. Chain file format:

```
#HOTEL;<chain name>;<file password hash, empty = no password>
#SITE;<city>            one per location, empty ones too
LastName;FirstName;Age;Room Number;City
<last>;<first>;<age>;<room>;<city>
```

`web/UFAZ_Hotels.txt` is a sample chain with the file password `ufaz2026`.

## Our rules

1. The 3 submitted files must compile alone: `gcc -Wall -Wextra submit/tp3.c submit/main.c`.
2. Code only the GUIs need goes in `web/`, never in `submit/`.
3. List functions don't use `scanf`; all the menu input is in `main.c`.
4. Every `malloc` needs a `free`. Run `make debug` before merging.
