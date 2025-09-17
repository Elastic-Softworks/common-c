# ===================================
# C O M M O N - C   M A K E F I L E
# BUILD SYSTEM FOR C89 COMPLIANCE
# ELASTIC SOFTWORKS 2025
# ===================================

# Compiler settings for strict C89 compliance
CC := gcc
CFLAGS := -std=c89 -pedantic -Wall -Wextra -Werror
CFLAGS += -Wstrict-prototypes -Wmissing-prototypes -Wold-style-definition
CFLAGS += -I./include

# Debug and release configurations
DEBUG_FLAGS := -g -O0 -DDEBUG
RELEASE_FLAGS := -O2 -DNDEBUG

# Directories
SRC_DIR := src
INCLUDE_DIR := include
BUILD_DIR := build
TEST_DIR := test

# Source files (excluding input.c until header is resolved)
SOURCES := $(SRC_DIR)/args.c \
           $(SRC_DIR)/audio.c \
           $(SRC_DIR)/error.c \
           $(SRC_DIR)/file.c \
           $(SRC_DIR)/graphics.c \
           $(SRC_DIR)/hash_table.c \
           $(SRC_DIR)/list.c \
           $(SRC_DIR)/math.c \
           $(SRC_DIR)/memory.c \
           $(SRC_DIR)/net.c \
           $(SRC_DIR)/queue.c \
           $(SRC_DIR)/stack.c \
           $(SRC_DIR)/string.c \
           $(SRC_DIR)/time.c \
           $(SRC_DIR)/tree.c \
           $(SRC_DIR)/vector.c

# Object files
OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Library output
LIBRARY := $(BUILD_DIR)/libcommc.a

# Default target
all: debug

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(LIBRARY)

# Release build  
release: CFLAGS += $(RELEASE_FLAGS)
release: $(LIBRARY)

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create static library
$(LIBRARY): $(OBJECTS)
	ar rcs $@ $^
	@echo "Library created: $(LIBRARY)"

# Test compilation (compile-only check)
test-compile: $(OBJECTS)
	@echo "All source files compiled successfully!"

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(SRC_DIR)/*.o

# Install headers and library (basic)
install: $(LIBRARY)
	@echo "Install target not yet implemented"

# Check for common issues
check:
	@echo "Checking for common issues..."
	@find . -name "*.o" -not -path "./$(BUILD_DIR)/*" && echo "Found stray object files!" || echo "No stray object files found."
	@find . -name "*.h" -exec grep -l "ifndef.*_H" {} \; | wc -l | xargs echo "Header files with guards:"
	@echo "Header count: $$(find $(INCLUDE_DIR) -name '*.h' | wc -l)"
	@echo "Source count: $$(find $(SRC_DIR) -name '*.c' | wc -l)"

# Force rebuild
rebuild: clean all

# Help
help:
	@echo "COMMON-C Build System"
	@echo "Available targets:"
	@echo "  all        - Build debug version (default)"
	@echo "  debug      - Build with debug symbols"
	@echo "  release    - Build optimized version"
	@echo "  test-compile - Test compilation without linking"
	@echo "  clean      - Remove build artifacts"
	@echo "  check      - Check for common issues"
	@echo "  rebuild    - Clean and rebuild"
	@echo "  help       - Show this help message"

.PHONY: all debug release test-compile clean install check rebuild help