#!/bin/bash
if [ $# -ne 2 ]; then
    echo "引数の数が正しくありません"
    exit 1
fi

input_file=$1
output_file=$2

g++ \
 -O3 -shared -std=c++23 -undefined dynamic_lookup \
 $(python3 -m pybind11 --includes) \
  $input_file -o $output_file

if [ $? -eq 0 ]; then
    echo "almo.so をビルドしました"
else
    echo "ビルドエラー"
fi

