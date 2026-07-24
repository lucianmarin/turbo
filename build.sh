#!/bin/bash
set -e

echo "=== Cleaning up previous build artifacts ==="
rm -f test.c test_bin test_stage2_bin stage1_turbo.c stage1_turbo stage2_turbo.c stage2_turbo

echo "=== Step 1: Compiling and running test.py using stage0 (python3) ==="
python3 turbo.py test.py -o test.c
gcc test.c runtime.c -o test_bin
./test_bin

echo "=== Step 2: Compiling compiler (turbo.py) using stage0 (python3) ==="
python3 turbo.py turbo.py -o stage1_turbo.c

echo "=== Step 3: Compiling stage1 C source to native binary ==="
gcc -O2 stage1_turbo.c runtime.c -o stage1_turbo

echo "=== Step 4: Compiling compiler (turbo.py) using stage1 binary ==="
./stage1_turbo turbo.py -o stage2_turbo.c

echo "=== Step 5: Comparing stage1_turbo.c and stage2_turbo.c ==="
if diff stage1_turbo.c stage2_turbo.c > /dev/null; then
    echo "SUCCESS: stage1 and stage2 generated C code are identical!"
    echo "Turbo is fully bootstrapped and self-hosting! 🎉"
else
    echo "FAILURE: stage1 and stage2 generated C code differ."
    diff -u stage1_turbo.c stage2_turbo.c | head -n 50
    exit 1
fi

echo "=== Step 6: Compiling stage2 binary ==="
gcc -O2 stage2_turbo.c runtime.c -o stage2_turbo

echo "=== Step 7: Verifying stage2 binary by compiling test.py ==="
./stage2_turbo test.py -o test_stage2.c
gcc test_stage2.c runtime.c -o test_stage2_bin
./test_stage2_bin

echo "=== Verification complete! ==="
