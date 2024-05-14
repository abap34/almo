.PHONY: setup

setup:
	bash setup.sh

build: setup
	g++ -std=c++23 build/almo.cpp -o build/almo

pybind: setup
	pipenv run bash pybind.sh

all: build pybind

clean:
	rm -rf build
	rm almo.so

