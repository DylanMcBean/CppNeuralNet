#
# Makefile for C++ Project with AFL and Fsanitizer
#


MAKEFLAGS += --no-print-directory

SRC_DIR = ../src
INC_DIR = ../include
TESTS_IN_DIR = ../tests/in
TESTS_OUT_DIR = ../tests/out
OBJ_DIR = ../build/bin/release/obj
BIN_DIR = ../build/bin/release
DEBUG_OBJ_DIR = ../build/bin/debug/obj
DEBUG_BIN_DIR = ../build/bin/debug
FUZZ_BIN_DIR = ../build/bin/fuzz

# Compiler Flags
CC = clang++
AFL_CC = afl-clang-fast++
CFLAGS = -Wall -c -O3 -I$(INC_DIR) -I/usr/include/opencv4 -std=c++20
DEBUG_CFLAGS = $(CFLAGS) -g
FUZZ_CFLAGS = -g -fsanitize=address,undefined

# List of Source Files
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp $(SRC_DIR)/**/*.cpp)

# List of Object Files
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))
DEBUG_OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(DEBUG_OBJ_DIR)/%.o, $(SRC_FILES))

# List of Dependency Files
DEP_FILES := $(wildcard $(INC_DIR)/*.hpp)


# Main Target
all: $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $(BIN_DIR)/main -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc

debug: $(DEBUG_OBJ_FILES) $(patsubst $(DEBUG_OBJ_DIR)/%.o, $(DEBUG_OBJ_DIR)/%.s, $(DEBUG_OBJ_FILES))
	$(CC) $(DEBUG_OBJ_FILES) -o $(DEBUG_BIN_DIR)/main -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc

fuzz:
	mkdir -p $(FUZZ_BIN_DIR)
	$(AFL_CC) $(FUZZ_CFLAGS) $(SRC_FILES) -o $(FUZZ_BIN_DIR)/main

%.s: %.o
	objdump -S -M intel $< > $@

# Compile Source Files
$(OBJ_FILES): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP_FILES)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

# Compile Source Files with Debugging Enabled
$(DEBUG_OBJ_FILES): $(DEBUG_OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP_FILES)
	mkdir -p $(dir $@)
	$(CC) $(DEBUG_CFLAGS) $< -o $@

# Generate assembly files from object files
$(DEBUG_OBJ_DIR)/%.s: $(DEBUG_OBJ_DIR)/%.o
	objdump -S $< > $@

# Clean Target
clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)/main $(DEBUG_OBJ_DIR) $(DEBUG_BIN_DIR)/main $(FUZZ_OBJ_DIR) $(FUZZ_BIN_DIR)/main $(TESTS_OUT_DIR)/*

run:
ifneq ($(wildcard $(BIN_DIR)/main),)
	@echo -e "\033[1;37;44mRelease Mode\033[0m"
	@echo
	@time "$(BIN_DIR)/main" "$(RUN_ARGS)"
else ifneq ($(wildcard $(DEBUG_BIN_DIR)/main),)
	@echo -e "\033[1;37;46mDebug Mode\033[0m"
	@echo
	@time "$(DEBUG_BIN_DIR)/main" "$(RUN_ARGS)"
else ifneq ($(wildcard $(FUZZ_BIN_DIR)/main),)
	@echo -e "\033[1;37;45mFuzz Mode\033[0m"
	@echo
	@afl-fuzz -i $(TESTS_IN_DIR) -o $(TESTS_OUT_DIR) $(FUZZ_BIN_DIR)/main @@
else
	@echo "No Builds built"
	@echo "Try running 'make (,debug,fuzz)' and then run this command again"
endif

# Default Target
.DEFAULT_GOAL := all

.PHONY: clean run