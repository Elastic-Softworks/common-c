# ===================================
# C O M M O N - C   M A K E F I L E
# BUILD SYSTEM FOR C89 COMPLIANCE
# ELASTIC SOFTWORKS 2025
# ===================================

# version information

VERSION_MAJOR := 0
VERSION_MINOR := 1
VERSION_PATCH := 0
VERSION := $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

# compiler settings for strict C89 compliance

CC := gcc
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

# directories

SRC_DIR := src
INCLUDE_DIR := include
BUILD_DIR := build
TEST_DIR := test

# source files

SOURCES := $(SRC_DIR)/args.c \
           $(SRC_DIR)/audio.c \
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
           $(SRC_DIR)/stack.c \
           $(SRC_DIR)/string.c \
           $(SRC_DIR)/time.c \
           $(SRC_DIR)/tree.c \
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
	@echo "Library created: $(LIBRARY)"

# test compilation (compile-only check)

test-compile: $(OBJECTS)
	@echo "All source files compiled successfully!"

# testing framework

TEST_SOURCES := $(wildcard $(TEST_DIR)/test_*.c)
TEST_EXECUTABLES := $(TEST_SOURCES:$(TEST_DIR)/%.c=$(BUILD_DIR)/%)

test: $(LIBRARY) $(TEST_EXECUTABLES)
	@echo "RUNNING TESTS..."
	@for test_exe in $(TEST_EXECUTABLES); do \
		./$$test_exe; \
	done

$(BUILD_DIR)/%: $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $@ $(LIBRARY)

# clean build artifacts

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(SRC_DIR)/*.o

# install headers and library (basic)

install: $(LIBRARY)
	@echo "Install target not yet implemented"

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
	@echo "  TEST-COMPILE  - TEST COMPILATION WITHOUT LINKING"
	@echo "  CLEAN         - REMOVE BUILD ARTIFACTS"
	@echo "  CHECK         - CHECK FOR COMMON ISSUES"
	@echo "  REBUILD       - CLEAN AND REBUILD"
	@echo "  HELP          - SHOW THIS HELP MESSAGE"

.PHONY: all debug release test test-compile clean install check rebuild help
