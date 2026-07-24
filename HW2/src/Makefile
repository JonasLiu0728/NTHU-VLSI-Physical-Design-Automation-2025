# ============================================================
#  Makefile for CS6135 HW2 - FM Partitioning
# ============================================================

SHELL := /bin/bash

# -------- Compiler & flags --------
CXX      = g++
CXXFLAGS = -std=c++11 -O2 -Wall -Wextra -Wno-sign-compare -Wno-unused-variable -Wno-unused-parameter

# -------- Paths (relative to src/) --------
BIN_DIR   := ../bin
BIN       := $(BIN_DIR)/hw2
IN_DIR    ?= ../testcase
OUT_DIR   ?= ../output
VERIFIER  := ../verifier/verify

# -------- Source files --------
SRCS := main.cpp graph.cpp fm.cpp
OBJS := $(SRCS:.cpp=.o)

# -------- Tools --------
TIME    ?= /usr/bin/time -f "time %E"
TIMEOUT ?= timeout 600

# -------- Phony --------
.PHONY: all clean run run4 verify verify4 run_verify run_verify4 build_dirs

# ============================================================
#  Build rules
# ============================================================

all: $(BIN)

$(BIN): build_dirs $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(BIN)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

build_dirs:
	@mkdir -p $(BIN_DIR) $(OUT_DIR)

# ============================================================
#  Run (k=2 / k=4)
# ============================================================

run: $(BIN) build_dirs
	@echo ">> public1 k=2";  $(TIME) $(TIMEOUT) $(BIN) $(IN_DIR)/public1.txt $(OUT_DIR)/public1_2.out 2
	@echo ">> public1 k=4";  $(TIME) $(TIMEOUT) $(BIN) $(IN_DIR)/public1.txt $(OUT_DIR)/public1_4.out 4
	@echo ">> public2 k=2";  $(TIME) $(TIMEOUT) $(BIN) $(IN_DIR)/public2.txt $(OUT_DIR)/public2_2.out 2
	@echo ">> public2 k=4";  $(TIME) $(TIMEOUT) $(BIN) $(IN_DIR)/public2.txt $(OUT_DIR)/public2_4.out 4
	@echo ">> public3 k=2";  $(TIME) $(TIMEOUT) $(BIN) $(IN_DIR)/public3.txt $(OUT_DIR)/public3_2.out 2;
	@echo ">> public3 k=4";  $(TIME) $(TIMEOUT) $(BIN) $(IN_DIR)/public3.txt $(OUT_DIR)/public3_4.out 4;
	@echo ">> public4 k=2";  $(TIME) $(TIMEOUT) $(BIN) $(IN_DIR)/public4.txt $(OUT_DIR)/public4_2.out 2;
	@echo ">> public4 k=4";  $(TIME) $(TIMEOUT) $(BIN) $(IN_DIR)/public4.txt $(OUT_DIR)/public4_4.out 4;
	@echo ">> public4 k=2";  $(TIME) $(TIMEOUT) $(BIN) $(IN_DIR)/public5.txt $(OUT_DIR)/public5_2.out 2;
	@echo ">> public4 k=4";  $(TIME) $(TIMEOUT) $(BIN) $(IN_DIR)/public5.txt $(OUT_DIR)/public5_4.out 4;
	@echo "---- Cuts ----"
	@grep -H -m1 '^CutSize' $(OUT_DIR)/*.out || true

run4: $(BIN) build_dirs
	@echo ">> k=4 only"
	@echo ">> public1 k=4";  $(TIME) $(TIMEOUT) $(BIN) $(IN_DIR)/public1.txt $(OUT_DIR)/public1_4.out 4
	@echo ">> public2 k=4";  $(TIME) $(TIMEOUT) $(BIN) $(IN_DIR)/public2.txt $(OUT_DIR)/public2_4.out 4
	@if [[ -f "$(IN_DIR)/public4.txt" ]]; then \
	  echo ">> public4 k=4";  $(TIME) $(TIMEOUT) $(BIN) $(IN_DIR)/public4.txt $(OUT_DIR)/public4_4.out 4; \
	fi

# ============================================================
#  Verify (k=2 / k=4)
# ============================================================

verify: build_dirs
	@chmod u+x $(VERIFIER) || true
	@echo ">> Verify public1 k=2"; $(VERIFIER) $(IN_DIR)/public1.txt $(OUT_DIR)/public1_2.out 2
	@echo ">> Verify public1 k=4"; $(VERIFIER) $(IN_DIR)/public1.txt $(OUT_DIR)/public1_4.out 4
	@echo ">> Verify public2 k=2"; $(VERIFIER) $(IN_DIR)/public2.txt $(OUT_DIR)/public2_2.out 2
	@echo ">> Verify public2 k=4"; $(VERIFIER) $(IN_DIR)/public2.txt $(OUT_DIR)/public2_4.out 4
	@echo ">> Verify public4 k=2"; $(VERIFIER) $(IN_DIR)/public4.txt $(OUT_DIR)/public4_2.out 2;
	@echo ">> Verify public4 k=4"; $(VERIFIER) $(IN_DIR)/public4.txt $(OUT_DIR)/public4_4.out 4;
	@echo ">> Verify public4 k=2"; $(VERIFIER) $(IN_DIR)/public5.txt $(OUT_DIR)/public5_2.out 2;
	@echo ">> Verify public4 k=4"; $(VERIFIER) $(IN_DIR)/public5.txt $(OUT_DIR)/public5_4.out 4;


run_verify: run verify
run_verify4: run4 verify4

# ============================================================
#  Utility
# ============================================================

clean:
	rm -f $(OBJS) $(BIN)
