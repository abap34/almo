.PHONY: setup

setup:
	bash scripts/setup.sh

build: setup
	g++ -std=c++23 build/almo.cpp -o build/almo

pybind: setup
	pipenv run bash scripts/pybind.sh

all: build pybind

clean:
	rm -rf build
	rm almo.so

