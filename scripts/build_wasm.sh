#!/bin/bash
set -euo pipefail

ALMO_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$ALMO_DIR/build_wasm"
SOURCE_DIR="$BUILD_DIR/src"

# Install the version in emscripten-version.txt with emsdk and activate its
# environment before running this script. Copy the paired outputs into consumers
# explicitly, after integration tests pass.
EXPECTED_VERSION=$(cat "$ALMO_DIR/scripts/emscripten-version.txt")
ACTUAL_VERSION=$(em++ -dumpversion)
if [[ "$ACTUAL_VERSION" != "$EXPECTED_VERSION" ]]; then
    echo "Expected Emscripten $EXPECTED_VERSION, found $ACTUAL_VERSION" >&2
    exit 1
fi

mkdir -p "$SOURCE_DIR"
cp -r "$ALMO_DIR/src/." "$SOURCE_DIR"

while IFS= read -r -d '' file; do
    printf 'R"(' > "$file.tmp"
    cat "$file" >> "$file.tmp"
    printf ')"\n' >> "$file.tmp"
    mv "$file.tmp" "$file"
done < <(find "$SOURCE_DIR" -type f \( -name '*.css' -o -name '*.html' -o -name '*.js' -o -name '*.txt' \) -print0)

VERSION=$(git -C "$ALMO_DIR" describe --tags --abbrev=0 2>/dev/null || echo "wasm-dev")
COMMITHASH=$(git -C "$ALMO_DIR" show --format='%h' --no-patch 2>/dev/null || echo "unknown")
printf 'R"(%s)"\n' "$VERSION" > "$SOURCE_DIR/version.txt"
printf 'R"(%s)"\n' "$COMMITHASH" > "$SOURCE_DIR/commithash.txt"

# -fexceptions is needed both when compiling and linking, so malformed Markdown
# can be reported as JSON and the same parser can process the next edit.
em++ -O2 -std=c++20 -fexceptions --no-entry \
    -I"$SOURCE_DIR" -I"$SOURCE_DIR/interfaces" -I"$SOURCE_DIR/syntax" \
    -s EXPORTED_FUNCTIONS='["_almo_render", "_almo_incremental_parser_create", "_almo_incremental_parser_set_source", "_almo_incremental_parser_destroy", "_malloc", "_free"]' \
    -s EXPORTED_RUNTIME_METHODS='["cwrap", "UTF8ToString", "lengthBytesUTF8", "stringToUTF8"]' \
    -s MODULARIZE=1 -s EXPORT_NAME='AlmoModule' \
    -s ENVIRONMENT=web,node -s ALLOW_MEMORY_GROWTH=1 \
    -o "$BUILD_DIR/almo.js" "$SOURCE_DIR/wasm_entry.cpp"

echo "Built $BUILD_DIR/almo.js and $BUILD_DIR/almo.wasm"
