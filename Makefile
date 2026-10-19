CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -g -Iinclude -MMD -MP
LDFLAGS =

BUILD_DIR = build
APP       = $(BUILD_DIR)/app
TEST_BIN  = $(BUILD_DIR)/test

# Every src/*.c is picked up automatically; main.c is left out of the tests
SRC      = $(wildcard src/*.c)
LIB_SRC  = $(filter-out src/main.c,$(SRC))
TEST_SRC = $(wildcard tests/*.c)

OBJ      = $(SRC:%.c=$(BUILD_DIR)/%.o)
LIB_OBJ  = $(LIB_SRC:%.c=$(BUILD_DIR)/%.o)
TEST_OBJ = $(TEST_SRC:%.c=$(BUILD_DIR)/%.o)

.PHONY: all run test debug clean

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

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJ:.o=.d) $(TEST_OBJ:.o=.d)
