#!/usr/bin/env bash
set -e

OUT="exe"
CU_FILES=$(find src -name "*.cu")
C_FILES=$(find src -name "*.c")
CPP_FILES=$(find src -name "*.cpp")

echo "Compiling: "
echo -e "\t $CU_FILES $C_FILES $CPP_FILES"

# nvcc $CU_FILES $CPP_FILES $C_FILES -o $OUT -O2
gcc $CPP_FILES $C_FILES -o $OUT -O2
