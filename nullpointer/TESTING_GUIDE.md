# Overflow Analysis Testing Guide

This guide shows you how to run and test the overflow analysis implementation.

## Quick Start

### 1. Enter the Docker Container

```bash
docker exec -it great_williams zsh
# or
docker exec -it great_williams bash
```

### 2. Navigate to Project Directory

```bash
cd /nullpointer
```

### 3. Build the Project (if not already built)

```bash
mkdir -p build
cd build
cmake ..
make -j4
cd ..
```

You should see:
- `[100%] Built target DivZeroPass`
- `[100%] Built target OverflowPass`

### 4. Run All Tests (Automated)

```bash
./run_overflow_tests.sh
```

This will run all overflow tests and show results with color coding.

---

## Manual Testing (Step by Step)

### Test a Single File

```bash
# Navigate to a test directory
cd /nullpointer/test/int_over_under_flow

# Compile C code to LLVM IR
clang -emit-llvm -S -fno-discard-value-names -Xclang -disable-O0-optnone -c -o test01.ll test01.c

# Run mem2reg optimization pass
opt -mem2reg -S test01.ll -o test01.opt.ll

# Run overflow analysis
opt -load-pass-plugin ../../build/OverflowPass.so -passes=Overflow test01.opt.ll -disable-output

# View the optimized LLVM IR (optional)
cat test01.opt.ll
```

### Using Make (Easier)

```bash
# Run all tests in a directory
cd /nullpointer/test/int_over_under_flow
make all

# Run a specific test
make test01

# View results
cat test01.out    # Overflow detection results
cat test01.err    # Detailed dataflow analysis

# Clean up generated files
make clean
```

---

## Test Directories

### 1. `test/int_over_under_flow/` - Basic Overflow Tests
Simple tests for integer overflow and underflow:
- `test01.c` - INT_MAX increment
- `test02.c` - Multiplication overflow
- `test03.c` - Underflow
- `test04.c` - Addition overflow
- `test05.c` - Subtraction underflow
- `test06.c` - Shift overflow

### 2. `test/CWE_190/` - CWE-190 Examples
Tests based on Common Weakness Enumeration 190 (Integer Overflow):
- `test13.c` through `test18.c` - Real-world overflow patterns

### 3. `test/more_data_types/` - Complex Data Types
Tests with different data types and scenarios:
- Various integer types
- More complex overflow scenarios

---

## Understanding Test Output

### Example: test01.c

**Input Code:**
```c
// Expect: fail
int main() {
  int p = __INT_MAX__;
  p++; // Error
  return 0;
}
```

**Running the test:**
```bash
cd /nullpointer/test/int_over_under_flow
make test01
cat test01.out
```

**Expected Output:**
```
Running Overflow on main
Potential Overflow Instructions by Overflow:
  %inc = add nsw i32 2147483647, 1
```

✅ This means the overflow was **detected**!

**Detailed Analysis (test01.err):**
```
Dataflow Analysis Results:
Instruction:   %inc = add nsw i32 2147483647, 1
In set:
Out set:
    [ %inc     |-> MaybeOverflow   ]
```

This shows the dataflow domain: `MaybeOverflow` indicates potential overflow.

---

## Running Specific Test Suites

### Run int_over_under_flow tests
```bash
cd /nullpointer/test/int_over_under_flow
make all
```

### Run CWE_190 tests
```bash
cd /nullpointer/test/CWE_190
make all
```

### Run more_data_types tests
```bash
cd /nullpointer/test/more_data_types
make all
```

---

## Analyzing Results

### Check which tests detected overflows:
```bash
cd /nullpointer/test/int_over_under_flow
grep -l "Potential Overflow" *.out
```

### View all detected overflows:
```bash
cd /nullpointer/test/int_over_under_flow
for f in *.out; do
    echo "=== $f ==="
    grep -A 5 "Potential Overflow" "$f"
done
```

### Count successful detections:
```bash
cd /nullpointer/test/int_over_under_flow
echo "Total tests: $(ls *.c | wc -l)"
echo "Overflows detected: $(grep -l "Potential Overflow" *.out 2>/dev/null | wc -l)"
```

---

## Troubleshooting

### Build failed?
```bash
cd /nullpointer/build
make clean
cmake ..
make -j4
```

### Tests not running?
Check that OverflowPass.so exists:
```bash
ls -lh /nullpointer/build/OverflowPass.so
```

### Permission errors?
Make sure you're in the container:
```bash
docker exec -it great_williams zsh
```

---

## Advanced: Viewing LLVM IR

To understand what the analysis is seeing:

```bash
cd /nullpointer/test/int_over_under_flow

# View original LLVM IR
cat test01.ll

# View optimized LLVM IR (after mem2reg)
cat test01.opt.ll

# The analysis runs on the .opt.ll file
```

---

## Quick Reference Commands

```bash
# Enter container
docker exec -it great_williams zsh

# Build project
cd /nullpointer && mkdir -p build && cd build && cmake .. && make -j4

# Run all tests
cd /nullpointer && ./run_overflow_tests.sh

# Run single test directory
cd /nullpointer/test/int_over_under_flow && make all

# View results
cat test01.out

# Clean up
make clean
```

---

## Next Steps

1. **Examine test failures** - Check which tests don't detect overflows
2. **Add more tests** - Create your own overflow test cases
3. **Improve analysis** - Enhance the overflow detection logic
4. **Compare with DivZero** - Run DivZero analysis on the same tests

---

For questions or issues, check the implementation in:
- `src/OverflowAnalysis.cpp` - Main analysis logic
- `src/Transfer.cpp` - Transfer functions
- `src/Domain.cpp` - Abstract domain operations
