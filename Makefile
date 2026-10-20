CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g -Iinclude -MMD -MP
LDFLAGS =

BUILD_DIR = build
APP       = $(BUILD_DIR)/app
TEST_BIN  = $(BUILD_DIR)/test
SUBMIT_DIR = submit

# Every src/*.c is picked up automatically; main.c is left out of the tests
SRC      = $(wildcard src/*.c)
LIB_SRC  = $(filter-out src/main.c,$(SRC))
TEST_SRC = $(wildcard tests/*.c)
# tp3.h first: every other header depends on its structs
HDR      = include/tp3.h $(filter-out include/tp3.h,$(wildcard include/*.h))

OBJ      = $(SRC:%.c=$(BUILD_DIR)/%.o)
LIB_OBJ  = $(LIB_SRC:%.c=$(BUILD_DIR)/%.o)
TEST_OBJ = $(TEST_SRC:%.c=$(BUILD_DIR)/%.o)

.PHONY: all run test debug clean submit

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

run: $(APP)
	./$(APP)

test: $(TEST_BIN)
	./$(TEST_BIN)

# Build into build/debug with sanitizers: crashes on leaks, overflows, use-after-free
debug:
	$(MAKE) BUILD_DIR=$(BUILD_DIR)/debug CFLAGS="$(CFLAGS) -fsanitize=address,undefined" LDFLAGS="-fsanitize=address,undefined" all test

# Merge the modules into the 3 files the statement asks for, then prove they compile on their own
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
