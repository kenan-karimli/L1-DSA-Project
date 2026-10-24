CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g -Isubmit
PYTHON ?= python3

TP3 = submit/tp3.c submit/tp3.h
WEB = web/web.c web/web.h

.PHONY: all run test debug lib gui web clean

all: build/app

build/app: $(TP3) submit/main.c
	@mkdir -p build
	$(CC) $(CFLAGS) submit/tp3.c submit/main.c -o $@

run: build/app
	./build/app

build/test_tp3: $(TP3) tests/test_tp3.c
	@mkdir -p build
	$(CC) $(CFLAGS) submit/tp3.c tests/test_tp3.c -o $@

build/test_web: $(TP3) $(WEB) web/test_web.c
	@mkdir -p build
	$(CC) $(CFLAGS) -Iweb submit/tp3.c web/web.c web/test_web.c -o $@

test: build/test_tp3 build/test_web
	./build/test_tp3
	./build/test_web

# same tests with AddressSanitizer: crashes on memory leaks or bad memory access
debug:
	$(MAKE) clean
	$(MAKE) test CFLAGS="$(CFLAGS) -fsanitize=address,undefined"
	$(MAKE) clean

# shared library for the Python GUIs (loaded with ctypes)
build/libhotel.so: $(TP3) $(WEB)
	@mkdir -p build
	$(CC) $(CFLAGS) -Iweb -fPIC -shared submit/tp3.c web/web.c -o $@

lib: build/libhotel.so

gui: build/libhotel.so
	$(PYTHON) web/hotel_gui.py

web: build/libhotel.so
	$(PYTHON) web/hotel_web.py

clean:
	rm -rf build
