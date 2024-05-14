#!/bin/bash

g++ \
 -O3 -shared -std=c++23 -undefined dynamic_lookup \
 $(python3 -m pybind11 --includes) \
 build/almo.cpp -o almo.so

if [ $? -eq 0 ]; then
    echo "almo.so をビルドしました"
else
    echo "ビルドエラー"
fi

