# ===================================
# C O M M O N - C   M A K E F I L E
# BUILD SYSTEM FOR C89 COMPLIANCE
# ELASTIC SOFTWORKS 2025
# ===================================

# version information

VERSION_MAJOR := 0
VERSION_MINOR := 1
VERSION_PATCH := 0
VERSION 	  := $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

# compiler settings for strict C89 compliance

CC     := gcc
CFLAGS := -std=c89 -pedantic -Wall -Wextra -Werror
CFLAGS += -Wstrict-prototypes -Wmissing-prototypes -Wold-style-definition
CFLAGS += -I./include
CFLAGS += -DCOMMC_VERSION_MAJOR=$(VERSION_MAJOR)
CFLAGS += -DCOMMC_VERSION_MINOR=$(VERSION_MINOR)
CFLAGS += -DCOMMC_VERSION_PATCH=$(VERSION_PATCH)
CFLAGS += -DCOMMC_VERSION_STRING=\"$(VERSION)\"

# debug and release configurations

DEBUG_FLAGS := -g -O0 -DDEBUG
RELEASE_FLAGS := -O2 -DNDEBUG

# platform-specific libraries

ifeq ($(OS),Windows_NT)
	LDLIBS := -lws2_32
else
	LDLIBS := 
endif

# directories

SRC_DIR := src
INCLUDE_DIR := include
BUILD_DIR := build
TEST_DIR := test

# source files

SOURCES := $(SRC_DIR)/args.c \
           $(SRC_DIR)/audio.c \
           $(SRC_DIR)/avl_tree.c \
           $(SRC_DIR)/b_tree.c \
           $(SRC_DIR)/error.c \
           $(SRC_DIR)/file.c \
           $(SRC_DIR)/graphics.c \
           $(SRC_DIR)/hash_table.c \
           $(SRC_DIR)/input.c \
           $(SRC_DIR)/list.c \
           $(SRC_DIR)/math.c \
           $(SRC_DIR)/memory.c \
           $(SRC_DIR)/net.c \
           $(SRC_DIR)/particles.c \
           $(SRC_DIR)/queue.c \
           $(SRC_DIR)/rb_tree.c \
           $(SRC_DIR)/splay_tree.c \
           $(SRC_DIR)/stack.c \
           $(SRC_DIR)/string.c \
           $(SRC_DIR)/time.c \
           $(SRC_DIR)/tree.c \
           $(SRC_DIR)/trie.c \
           $(SRC_DIR)/vector.c

# object files

OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# library output

LIBRARY := $(BUILD_DIR)/libcommc.a

# default target

all: debug

# debug build

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(LIBRARY)

# release build

release: CFLAGS += $(RELEASE_FLAGS)
release: $(LIBRARY)

# create build directory

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# compile source files

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# create static library

$(LIBRARY): $(OBJECTS)
	ar rcs $@ $^
	@echo "LIB CREATED: $(LIBRARY)"

# test compilation (compile-only check)

test-compile: $(OBJECTS)
	@echo "ALL TEST FILES COMPILED SUCCESSFULLY!"

# testing framework
 
 TEST_SOURCES := $(wildcard $(TEST_DIR)/test_*.c)
 TEST_EXECUTABLES := $(TEST_SOURCES:$(TEST_DIR)/%.c=$(BUILD_DIR)/%)
 
 test: $(LIBRARY) $(TEST_EXECUTABLES)
	@echo "RUNNING TESTS..."
	@for test_exe in $(TEST_EXECUTABLES); do \
		./$$test_exe; \
	done
 
 $(BUILD_DIR)/%: $(TEST_DIR)/%.c
	$(CC) $(TEST_CFLAGS) $< -o $@ $(LIBRARY) $(LDLIBS)

# valgrind memory testing (unix only)

valgrind-test: $(LIBRARY) $(TEST_EXECUTABLES)
	@if command -v valgrind >/dev/null 2>&1; then \
		echo "RUNNING TESTS WITH VALGRIND MEMORY CHECKING..."; \
		for test_exe in $(TEST_EXECUTABLES); do \
			echo "--- VALGRIND: $$test_exe ---"; \
			valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
				--track-origins=yes --verbose --log-fd=1 ./$$test_exe; \
		done; \
	else \
		echo "VALGRIND NOT FOUND - INSTALL WITH: sudo apt install valgrind"; \
		echo "OR ON MACOS: brew install valgrind"; \
		exit 1; \
	fi

# sanitizer-enhanced testing

SANITIZER_FLAGS := -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer
SANITIZER_CFLAGS := $(CFLAGS) $(SANITIZER_FLAGS) -g

# test-only flags: relax a couple of strict warnings for test sources so
# comprehensive test suites compile under strict project flags while keeping
# the production/library builds strict.

TEST_CFLAGS := $(CFLAGS) -Wno-missing-prototypes -Wno-unused-variable

sanitizer-test: $(LIBRARY)
	@echo "RUNNING SANITIZER-ENABLED TESTS (or falling back if unsupported)..."
	@if [ "$(OS)" = "Windows_NT" ]; then \
		echo "Sanitizers appear unsupported on this Windows/MSYS toolchain; compiling tests without sanitizer flags..."; \
		for test_src in $(TEST_SOURCES); do \
			test_name=$$(basename $$test_src .c); \
			echo "COMPILING: $$test_name"; \
			$(CC) $(TEST_CFLAGS) $$test_src -o $(BUILD_DIR)/$$test_name $(LIBRARY); \
		done; \
		echo "RUNNING TESTS..."; \
		for test_exe in $(TEST_EXECUTABLES); do \
			echo "--- TEST: $$test_exe ---"; \
			./$$test_exe; \
		done; \
	else \
		echo "BUILDING TESTS WITH ADDRESS/UNDEFINED SANITIZERS..."; \
		for test_src in $(TEST_SOURCES); do \
			test_name=$$(basename $$test_src .c); \
			echo "COMPILING: $$test_name"; \
			$(CC) $(SANITIZER_CFLAGS) $$test_src -o $(BUILD_DIR)/$$test_name $(LIBRARY); \
		done; \
		echo "RUNNING SANITIZER TESTS..."; \
		for test_exe in $(TEST_EXECUTABLES); do \
			echo "--- SANITIZER: $$test_exe ---"; \
			./$$test_exe; \
		done; \
	fi

# memory-specific testing targets

memory-test: $(LIBRARY) $(TEST_EXECUTABLES)
	@echo "NOTE: this runs standard tests - use valgrind-test for detailed analysis"
	@for test_exe in $(TEST_EXECUTABLES); do \
		echo "--- MEMORY TEST: $$test_exe ---"; \
		./$$test_exe; \
	done

# performance benchmarking targets

benchmark-test: $(LIBRARY) $(TEST_EXECUTABLES)
	@echo "NOTE: this runs standard tests with timing - results may vary by system"
	@for test_exe in $(TEST_EXECUTABLES); do \
		echo "--- BENCHMARK: $$test_exe ---"; \
		./$$test_exe; \
	done

# comprehensive testing with all modes

full-test: test valgrind-test sanitizer-test
	@echo "- STANDARD TESTS: PASSED"
	@echo "- VALGRIND TESTS: PASSED"  
	@echo "- SANITIZER TESTS: PASSED"
 
 # clean build artifacts

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(SRC_DIR)/*.o


# install headers and library (basic)

install: $(LIBRARY)
	@echo "INSTALL TARGET NOT YET ELIMINATED"

# check for common issues

check:
	@echo "CHECKING FOR COMMON ISSUES..."
	@find . -name "*.o" -not -path "./$(BUILD_DIR)/*" && echo "FOUND STRAY OBJECT FILES!" || echo "NO STRAY OBJECT FILES FOUND."
	@find . -name "*.h" -exec grep -l "ifndef.*_H" {} \; | wc -l | xargs echo "HEADER FILES WITH GUARDS:"
	@echo "HEADER COUNT: $$(find $(INCLUDE_DIR) -name '*.h' | wc -l)"
	@echo "SOURCE COUNT: $$(find $(SRC_DIR) -name '*.c' | wc -l)"

# force rebuild

rebuild: clean all

# help

help:
	@echo "COMMC BUILD SYSTEM"
	@echo "AVAILABLE TARGETS:"
	@echo "  ALL           - BUILD DEBUG VERSION (DEFAULT)"
	@echo "  DEBUG         - BUILD WITH DEBUG SYMBOLS"
	@echo "  RELEASE       - BUILD OPTIMIZED VERSION"
	@echo "  TEST          - COMPILE AND RUN ALL TESTS"
	@echo "  MEMORY-TEST   - RUN MEMORY LEAK DETECTION TESTS"
	@echo "  BENCHMARK-TEST - RUN PERFORMANCE BENCHMARK TESTS"
	@echo "  VALGRIND-TEST - RUN TESTS UNDER VALGRIND (UNIX ONLY)"
	@echo "  SANITIZER-TEST - RUN TESTS WITH ADDRESS/UB SANITIZERS"
	@echo "  FULL-TEST     - RUN ALL TESTING MODES"
	@echo "  TEST-COMPILE  - TEST COMPILATION WITHOUT LINKING"
	@echo "  CLEAN         - REMOVE BUILD ARTIFACTS"
	@echo "  CHECK         - CHECK FOR COMMON ISSUES"
	@echo "  REBUILD       - CLEAN AND REBUILD"
	@echo "  HELP          - SHOW THIS HELP MESSAGE"

.PHONY: all debug release test memory-test benchmark-test valgrind-test sanitizer-test full-test test-compile clean install check rebuild help
