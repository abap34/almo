#!/bin/bash
# ワイルドカードが展開されるとカレントディレクトリのファイル名で展開されてマッチがおかしくなるので展開を抑制する.
# たとえばカレントディレクトリに `tmp.html` があると pettern が "tmp.html" と展開されてマッチしてしまうので抑制する必要がある。
# 今の set -f の状態を保存しておいて、最後に元に戻す
mode=$(set -o | grep noglob | awk '{print $2}')
set -f

src_dir="src"
build_dir="build"
data_petterns=("*.css" "*.html" "*.js" "*.txt")
verbose=0

# 指定された内容を指定色で表示する. 
# 指定なければ デフォルト、　赤か緑を指定できる
function vprint() {
    if [ $verbose -eq 1 ]; then
        if [ $2 ]; then
            if [ $2 == "green" ]; then
                echo -e "\033[32m$1\033[0m"
            elif [ $2 == "red" ]; then
                echo -e "\033[31m$1\033[0m"
            else
                echo -e "\033[31m$1\033[0m"
            fi
        else
            echo $1
        fi
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

mkdir -p "$build_dir"
cp -r "$src_dir"/. "$build_dir"


for file in $(find $build_dir -type f); do  
    for pettern in ${data_petterns[@]}; do
        if [[ $file == $build_dir/*$pettern ]]; then
            vprint "   Matched! with $pettern => $file" green
            echo -n "R\"(" > "$file.tmp"
            cat "$file" >> "$file.tmp"
            echo ")\"" >> "$file.tmp"
            mv "$file.tmp" "$file"
            continue 2
        fi
    done

    vprint "$file is not matching any data file"
done

# バージョン、コミットハッシュの情報を出力
echo "R\"($(git describe --tags --abbrev=0))\"" > "$build_dir/version.txt"
echo "R\"($(git show --format='%h' --no-patch))\"" > "$build_dir/commithash.txt"


# 元に戻す
if [ $mode == "on" ]; then
    set +f
fi


vprint "Finish setup."
