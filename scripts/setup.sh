#!/bin/bash
# ワイルドカードが展開されるとカレントディレクトリのファイル名で展開されてマッチがおかしくなるので展開を抑制する.
# たとえばカレントディレクトリに `tmp.html` があると pettern が "tmp.html" と展開されてマッチしてしまうので抑制する必要がある。
# 今の set -f の状態を保存しておいて、最後に元に戻す
mode=$(set -o | grep noglob | awk '{print $2}')
set -f

src_dir="src/"
build_dir="build/"
data_petterns=("*.css" "*.html" "*.js")
verbose=0


function vprint() {
    if [ $verbose -eq 1 ]; then
        echo "$1"
    fi
}

while getopts "v" opt; do
    case $opt in
        v)
            verbose=1
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            exit 1
            ;;
    esac
done


vprint "========================================"
vprint "Start setup"
vprint "    src_dir: $src_dir"
vprint "    build_dir: $build_dir"
vprint "    data_petterns: ${data_petterns[@]}"
vprint "========================================"


mkdir -p "$build_dir"
cp -r "$src_dir" "$build_dir"


for file in $(find $build_dir -type f); do  
    vprint "----------------------------------------"
    vprint "Check $file for data files"
    for pettern in ${data_petterns[@]}; do
        vprint  "   Check $pettern"
        if [[ $file == $build_dir/*$pettern ]]; then
            vprint "$file is matching $pettern"
            echo "R\"(" > "$file.tmp"
            cat "$file" >> "$file.tmp"
            echo ")\"" >> "$file.tmp"
            mv "$file.tmp" "$file"
            continue 2
        fi
    done

    vprint "$file is not matching any data file"
done


# 元に戻す
if [ $mode == "on" ]; then
    set +f
fi


vprint "Finish setup."
