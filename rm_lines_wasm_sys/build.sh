#!/bin/bash
set -e
cd "$(dirname "$0")/.."
cmake --build build-web --target rm_lines_wasm

(
cd rm_lines_wasm_sys
cp ../build-web/rm_lines_wasm.js .
cp ../build-web/rm_lines_wasm.wasm .
cp ../build-web/rm_lines_wasm.wasm.map .

npm pack --dry-run
)