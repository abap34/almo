.PHONY: setup

setup:
	bash scripts/setup.sh

build: setup
	g++ -std=c++20 build/almo.cpp -o build/almo

pybind: setup
	bash scripts/pybind.sh build/pyalmo.cpp almo.so

all: clean build pybind

clean:
	rm -rf build
	rm -f almo.so

