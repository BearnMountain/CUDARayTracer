#!/usr/bin/env bash
set -e

OUT="exe"
CU_FILES=$(find src -name "*.cu")
C_FILES=$(find src -name "*.c")

echo "Compiling: "
echo "\t $CU_FILES $C_FILES"

nvcc $CU_FILES $C_FILES -o $OUT -O2
