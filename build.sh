#!/bin/bash
set -e

echo "=== Cleaning up previous build artifacts ==="
rm -f test.ll test.c test_bin test_stage2.ll test_stage2.c test_stage2_bin \
      stage1_turbo.ll stage1_turbo.c stage1_turbo \
      stage2_turbo.ll stage2_turbo.c stage2_turbo \
      stage3_turbo.ll stage3_turbo.c

echo "=== Step 1: Compiling and running test.py using stage0 (python3) ==="
python3 turbo.py test.py -o test.ll
clang -O2 -Wno-override-module test.ll runtime.c -lm -o test_bin
./test_bin

echo "=== Step 2: Compiling compiler (turbo.py) using stage0 (python3) ==="
python3 turbo.py turbo.py -o stage1_turbo.ll

echo "=== Step 3: Compiling stage1 LLVM IR (.ll) to native binary ==="
clang -O2 -Wno-override-module stage1_turbo.ll runtime.c -lm -o stage1_turbo

echo "=== Step 4: Compiling compiler (turbo.py) using stage1 binary ==="
./stage1_turbo turbo.py -o stage2_turbo.ll

echo "=== Step 5: Comparing stage1_turbo.ll and stage2_turbo.ll ==="
if diff stage1_turbo.ll stage2_turbo.ll > /dev/null; then
    echo "SUCCESS: stage1 and stage2 generated LLVM IR (.ll) are identical!"
else
    echo "FAILURE: stage1 and stage2 generated LLVM IR (.ll) differ."
    diff -u stage1_turbo.ll stage2_turbo.ll | head -n 50
    exit 1
fi

echo "=== Step 6: Compiling stage2 binary ==="
clang -O2 -Wno-override-module stage2_turbo.ll runtime.c -lm -o stage2_turbo

echo "=== Step 7: Compiling compiler (turbo.py) using stage2 binary (Stage 3) ==="
./stage2_turbo turbo.py -o stage3_turbo.ll
if diff stage2_turbo.ll stage3_turbo.ll > /dev/null; then
    echo "SUCCESS: stage2 and stage3 generated LLVM IR (.ll) are identical!"
    echo "Turbo is fully bootstrapped and self-hosting! 🎉"
else
    echo "FAILURE: stage2 and stage3 generated LLVM IR (.ll) differ."
    diff -u stage2_turbo.ll stage3_turbo.ll | head -n 50
    exit 1
fi

echo "=== Step 8: Verifying stage2 binary by compiling test.py ==="
./stage2_turbo test.py -o test_stage2.ll
clang -O2 -Wno-override-module test_stage2.ll runtime.c -lm -o test_stage2_bin
./test_stage2_bin

echo "=== Verification complete! ==="

