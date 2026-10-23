CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g -Iinclude -MMD -MP
LDFLAGS =

BUILD_DIR = build
APP       = $(BUILD_DIR)/app
TEST_BIN  = $(BUILD_DIR)/test
SUBMIT_DIR = submit
LIB_SO    = $(BUILD_DIR)/libhotel.so
PYTHON   ?= python3

# all .c files in src/ get compiled, main.c is not used for tests (tests have their own main)
SRC      = $(wildcard src/*.c)
LIB_SRC  = $(filter-out src/main.c,$(SRC))
TEST_SRC = $(wildcard tests/*.c)
# tp3.h has to be first, the other headers need the structs
HDR      = include/tp3.h $(filter-out include/tp3.h,$(wildcard include/*.h))

OBJ      = $(SRC:%.c=$(BUILD_DIR)/%.o)
LIB_OBJ  = $(LIB_SRC:%.c=$(BUILD_DIR)/%.o)
TEST_OBJ = $(TEST_SRC:%.c=$(BUILD_DIR)/%.o)

.PHONY: all run test debug clean submit lib gui web

all: $(APP)

$(APP): $(OBJ)
	@test -n "$(SRC)" || { echo "No .c files in src/ yet"; exit 1; }
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(TEST_BIN): $(LIB_OBJ) $(TEST_OBJ)
	@test -n "$(TEST_SRC)" || { echo "No .c files in tests/ yet"; exit 1; }
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# .so for the python GUI (it loads it with ctypes)
$(LIB_SO): $(LIB_SRC) $(wildcard include/*.h)
	@mkdir -p $(dir $@)
	$(CC) -Wall -Wextra -std=c11 -Iinclude -fPIC -shared $(LIB_SRC) -o $@

lib: $(LIB_SO)

# needs python with tkinter, for me: make gui PYTHON=~/miniconda3/bin/python3
gui: $(LIB_SO)
	$(PYTHON) gui/hotel_gui.py

# web version, opens on http://127.0.0.1:8000 (no tkinter needed)
web: $(LIB_SO)
	$(PYTHON) gui/hotel_web.py

run: $(APP)
	./$(APP)

test: $(TEST_BIN)
	./$(TEST_BIN)

# debug build with address sanitizer, it crashes if we have leaks / bad memory access
debug:
	$(MAKE) BUILD_DIR=$(BUILD_DIR)/debug CFLAGS="$(CFLAGS) -fsanitize=address,undefined" LDFLAGS="-fsanitize=address,undefined" all test

# we have to submit only tp3.h, tp3.c and main.c so this glues all our files together
# and compiles them to make sure it still works
submit:
	@rm -rf $(SUBMIT_DIR) && mkdir -p $(SUBMIT_DIR) $(BUILD_DIR)
	{ echo '#pragma once'; for f in $(HDR); do echo "/* ---- $$f ---- */"; sed '/^#pragma once/d; /^#include "/d' $$f; done; } > $(SUBMIT_DIR)/tp3.h
	{ echo '#include "tp3.h"'; for f in $(LIB_SRC); do echo "/* ---- $$f ---- */"; sed '/^#include "/d' $$f; done; } > $(SUBMIT_DIR)/tp3.c
	{ echo '#include "tp3.h"'; sed '/^#include "/d' src/main.c; } > $(SUBMIT_DIR)/main.c
	$(CC) -Wall -Wextra -std=c11 $(SUBMIT_DIR)/tp3.c $(SUBMIT_DIR)/main.c -o $(BUILD_DIR)/submit_check
	@echo "Ready to upload: $(SUBMIT_DIR)/tp3.h $(SUBMIT_DIR)/tp3.c $(SUBMIT_DIR)/main.c"

clean:
	rm -rf $(BUILD_DIR) $(SUBMIT_DIR)

-include $(OBJ:.o=.d) $(TEST_OBJ:.o=.d)
