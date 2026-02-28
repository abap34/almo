#!/bin/bash
set -e

ALMO_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$ALMO_DIR/build_wasm"
CMS_DIR="${CMS_DIR:-$ALMO_DIR/../cms.abap34.com}"
OUTPUT_DIR="$CMS_DIR/public/almo"

echo "=== almo wasm build ==="
echo "ALMO_DIR: $ALMO_DIR"
echo "BUILD_DIR: $BUILD_DIR"
echo "OUTPUT_DIR: $OUTPUT_DIR"

# Clean and create build directory
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Copy source files
cp -r "$ALMO_DIR/src/." "$BUILD_DIR"

# Wrap CSS/HTML/JS/TXT files as C++ raw string literals (same as setup.sh)
mode=$(set -o | grep noglob | awk '{print $2}')
set -f

data_patterns=("*.css" "*.html" "*.js" "*.txt")

for file in $(find "$BUILD_DIR" -type f); do
    for pattern in "${data_patterns[@]}"; do
        if [[ $file == $BUILD_DIR/*$pattern ]]; then
            echo "  Wrapping: $file"
            echo -n 'R"(' > "$file.tmp"
            cat "$file" >> "$file.tmp"
            echo ')"' >> "$file.tmp"
            mv "$file.tmp" "$file"
            continue 2
        fi
    done
done

if [ "$mode" == "on" ]; then
    set +f
fi

# Create version/commithash files
cd "$ALMO_DIR"
VERSION=$(git describe --tags --abbrev=0 2>/dev/null || echo "wasm-dev")
COMMITHASH=$(git show --format='%h' --no-patch 2>/dev/null || echo "unknown")
echo "R\"($VERSION)\"" > "$BUILD_DIR/version.txt"
echo "R\"($COMMITHASH)\"" > "$BUILD_DIR/commithash.txt"

# Compile with emscripten
echo "=== Compiling with emcc ==="
emcc -O2 \
    -std=c++20 \
    -I"$BUILD_DIR" \
    -I"$BUILD_DIR/interfaces" \
    -I"$BUILD_DIR/syntax" \
    -s EXPORTED_FUNCTIONS='["_almo_render", "_malloc", "_free"]' \
    -s EXPORTED_RUNTIME_METHODS='["cwrap", "UTF8ToString", "lengthBytesUTF8", "stringToUTF8"]' \
    -s MODULARIZE=1 \
    -s EXPORT_NAME='AlmoModule' \
    -s ENVIRONMENT=web \
    -s ALLOW_MEMORY_GROWTH=1 \
    -o "$BUILD_DIR/almo.js" \
    "$BUILD_DIR/wasm_entry.cpp"

echo "=== Build complete ==="

# Copy output to CMS public directory
mkdir -p "$OUTPUT_DIR"
cp "$BUILD_DIR/almo.js" "$OUTPUT_DIR/almo.js"
cp "$BUILD_DIR/almo.wasm" "$OUTPUT_DIR/almo.wasm"

echo "=== Copied to $OUTPUT_DIR ==="
ls -la "$OUTPUT_DIR"
