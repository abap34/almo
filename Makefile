.PHONY: setup build pybind all clean test bench coverage

CXX ?= g++
CXXFLAGS ?= -std=c++20
TEST_CXXFLAGS ?= $(CXXFLAGS) -O0 -g
BENCH_CXXFLAGS ?= $(CXXFLAGS) -O2 -g
COVERAGE_CXXFLAGS ?= $(CXXFLAGS) -O0 -g --coverage

TEST_BINARY := build/almo_tests
BENCH_BINARY := build/almo_bench
COVERAGE_BINARY := build/almo_tests_coverage
COVERAGE_DIR := build/coverage
COVERAGE_INFO := $(COVERAGE_DIR)/coverage.info
GCOV ?= gcov-$(shell $(CXX) -dumpversion | cut -d. -f1)
BENCH_ARGS ?=

setup:
	bash scripts/setup.sh

build: setup
	$(CXX) $(CXXFLAGS) build/almo.cpp -o build/almo

pybind: setup
	bash scripts/pybind.sh build/pyalmo.cpp almo.so

test: setup
	$(CXX) $(TEST_CXXFLAGS) tests/test_main.cpp -o $(TEST_BINARY)
	./$(TEST_BINARY)

bench: setup
	$(CXX) $(BENCH_CXXFLAGS) tests/benchmark_main.cpp -o $(BENCH_BINARY)
	./$(BENCH_BINARY) $(BENCH_ARGS)

coverage: setup
	mkdir -p $(COVERAGE_DIR)
	rm -f build/*.gcda build/*.gcno
	$(CXX) $(COVERAGE_CXXFLAGS) tests/test_main.cpp -o $(COVERAGE_BINARY)
	./$(COVERAGE_BINARY)
	lcov --gcov-tool $(GCOV) --capture --directory build --output-file $(COVERAGE_INFO)
	lcov --ignore-errors unused --remove $(COVERAGE_INFO) '/usr/*' '/Library/*' '/opt/homebrew/*' '*/tests/*' --output-file $(COVERAGE_INFO)
	lcov --summary $(COVERAGE_INFO)
	genhtml $(COVERAGE_INFO) --output-directory $(COVERAGE_DIR)/html

all: clean build pybind

clean:
	rm -rf build
	rm -f almo.so
