# Quick Start Guide - Overflow Analysis Testing

## TL;DR - Just Run This

```bash
# Enter the container
docker exec -it great_williams zsh

# Navigate to project
cd /nullpointer

# Run interactive menu
./test_menu.sh
```

Select option `1` to run all tests automatically!

---

## Option 1: Interactive Menu (Easiest)

```bash
docker exec -it great_williams zsh
cd /nullpointer
./test_menu.sh
```

Then choose:
- `1` - Run ALL tests (recommended first time)
- `2-4` - Run specific test suites
- `5` - Run a single test manually
- `6` - View test source code
- `7` - Calculate Precision & Recall metrics
- `8` - Clean all test outputs
- `9` - Rebuild OverflowPass
- `h` - Help

---

## Option 2: Automated Script

```bash
docker exec -it great_williams zsh
cd /nullpointer
./run_overflow_tests.sh
```

This runs all tests, shows a summary with color coding, and automatically calculates precision/recall metrics.

---

## Precision & Recall Metrics

After running tests, you'll automatically see metrics like:

```
============================================================
INTEGER OVERFLOW DETECTION - PRECISION & RECALL
============================================================

Total Tests:        18
True Positives:     8
False Positives:    1
True Negatives:     7
False Negatives:    2

------------------------------------------------------------
Precision:          88.89%
Recall:             80.00%
F1 Score:           84.21%
Accuracy:           83.33%
------------------------------------------------------------
```

To view metrics on existing test results:
```bash
cd /nullpointer
python3 calculate_metrics.py --verbose
```

See `METRICS_GUIDE.md` for detailed documentation.

---

## Option 3: Manual Commands

```bash
# Enter container
docker exec -it great_williams zsh

# Go to a test directory
cd /nullpointer/test/int_over_under_flow

# Run all tests in this directory
make all

# Run specific test
make test01

# View results
cat test01.out    # Overflow detection
cat test01.err    # Dataflow analysis
```

---

## What You Should See

### Successful Detection Example:
```
Running Overflow on main
Potential Overflow Instructions by Overflow:
  %inc = add nsw i32 2147483647, 1
```

This means the overflow was **DETECTED** ✓

### No Detection Example:
```
Running Overflow on main
Potential Overflow Instructions by Overflow:

```

No instructions listed = no overflow detected

---

## Test Directories

| Directory | Description | Test Count |
|-----------|-------------|------------|
| `test/int_over_under_flow/` | Basic overflow/underflow tests | 6 tests |
| `test/CWE_190/` | CWE-190 real-world examples | 6 tests |
| `test/more_data_types/` | Complex data type tests | ~10 tests |

---

## Common Tasks

### Run all tests quickly:
```bash
cd /nullpointer && ./run_overflow_tests.sh
```

### Test one file step-by-step:
```bash
cd /nullpointer/test/int_over_under_flow

# Compile to LLVM IR
clang -emit-llvm -S -fno-discard-value-names -Xclang -disable-O0-optnone -c -o test01.ll test01.c

# Optimize
opt -mem2reg -S test01.ll -o test01.opt.ll

# Run analysis
opt -load-pass-plugin ../../build/OverflowPass.so -passes=Overflow test01.opt.ll -disable-output
```

### View all detected overflows:
```bash
cd /nullpointer/test/int_over_under_flow
grep -H "Potential Overflow" *.out
```

### Count detection rate:
```bash
cd /nullpointer/test/int_over_under_flow
total=$(ls *.c | wc -l)
detected=$(grep -l "Potential Overflow" *.out 2>/dev/null | wc -l)
echo "Detected $detected out of $total tests"
```

---

## Need More Help?

- **Full Guide**: `cat /nullpointer/TESTING_GUIDE.md`
- **Implementation**: Check `src/OverflowAnalysis.cpp`
- **Interactive Menu**: `./test_menu.sh` (option 9 for help)

---

## Troubleshooting

**"OverflowPass.so not found"**
```bash
# Make sure you're in the container first!
docker exec -it great_williams zsh
cd /nullpointer

# If build directory exists with wrong config, remove it
rm -rf build

# Build from scratch
mkdir -p build && cd build
cmake ..
make -j4

# Verify build succeeded
ls -la OverflowPass.so
```

**Permission errors**
```bash
# Make sure you're in the container
docker exec -it great_williams zsh
```

**Can't find scripts**
```bash
# Make sure they're executable
cd /nullpointer
chmod +x run_overflow_tests.sh test_menu.sh
```

---

## Next Steps

1. ✅ Run all tests: `./run_overflow_tests.sh`
2. 📊 Review results in `.out` files
3. 🔍 Examine false negatives (should detect but doesn't)
4. 🔧 Improve the analysis in `src/OverflowAnalysis.cpp`
5. ➕ Add your own test cases

Happy testing! 🚀
