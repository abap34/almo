#!/bin/bash

src_dir="src/"

build_dir="build/"

mkdir -p "$build_dir"

for file in "$src_dir"*
do
    cp "$file" "$build_dir"
    echo "ファイル $file を $build_dir にコピーしました。"
done


for file in "$build_dir"*
do
    extension="${file##*.}"

    if [ "$extension" != "cpp" ] && [ "$extension" != "hpp" ]; then
        sed -i '' '1s/^/R"(/' "$file"

        echo ')"' >> "$file"

        echo "ファイル $file を処理済み"
    fi

done

g++ -std=c++20 "$build_dir"almo.cpp -o "$build_dir"almo 

mv "$build_dir"almo .

echo "ビルドが完了しました。"
