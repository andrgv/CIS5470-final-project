# CIS 5470 Final Project: Integer Overflow & Null Pointer Detection

**Team Members:** Amand and Andrea

## Overview

This project implements static analysis tools to detect two critical bug classes in C programs:

1. **Integer Overflow/Underflow Detection** - Using abstract interpretation with interval domains and dataflow analysis
2. **Null Pointer Dereference Detection** - Using dataflow analysis with pointer aliasing support

The analyses are implemented as LLVM passes that operate on LLVM IR, enabling precise program analysis through forward intraprocedural dataflow analysis.

## Repository Layout

```
nullpointer/
├── CMakeLists.txt              # CMake build configuration
├── Makefile                    # Top-level build commands
├── README.md                   # This file
├── QUICKSTART.md              # Quick start guide for running tests
├── TESTING_GUIDE.md           # Detailed testing documentation
├── METRICS_GUIDE.md           # Precision & Recall metrics guide
│
├── include/                    # Header files
│   ├── OverflowAnalysis.h     # Integer overflow analysis
│   ├── DomainOverflow.h       # Interval domain for overflow
│   ├── NullPointerAnalysis.h  # Null pointer analysis
│   ├── PointerAnalysis.h      # Pointer aliasing analysis
│   ├── Domain.h               # Abstract domain definitions
│   └── Utils.h                # Utility functions
│
├── src/                        # Implementation files
│   ├── OverflowAnalysis.cpp   # Overflow detection pass
│   ├── DomainOverflow.cpp     # Interval domain operations
│   ├── NullPointerAnalysis.cpp # Null pointer detection pass
│   ├── PointerAnalysis.cpp    # Pointer analysis implementation
│   ├── ChaoticIteration.cpp   # Chaotic iteration for fixpoint
│   ├── Transfer.cpp           # Transfer functions
│   ├── Domain.cpp             # Domain operations
│   └── Utils.cpp              # Utilities
│
├── test/                       # Test suites
│   ├── int_over_under_flow/   # Basic overflow tests (6 tests)
│   ├── CWE_190/               # CWE-190 real-world tests (6+ tests)
│   ├── more_data_types/       # Complex data type tests (~10 tests)
│   └── nullpointer/           # Null pointer tests (~18 tests)
│       └── aliasing/          # Pointer aliasing tests
│
├── doc/                        # Documentation
│   └── project_proposal.md    # Original project proposal
│
├── build/                      # Build artifacts (generated)
│   ├── OverflowPass.so        # Overflow analysis LLVM pass
│   └── NullPtrPass.so         # Null pointer analysis pass
│
└── Scripts
    ├── run_overflow_tests.sh  # Run all overflow tests
    ├── test_menu.sh           # Interactive test menu
    ├── test_with_timeout.sh   # Test runner with timeout
    └── calculate_metrics.py   # Calculate precision/recall metrics
```

## Technical Approach

### Integer Overflow Detection

**Technique:** Abstract Interpretation with Interval Domain
- Tracks integer value ranges using interval domain [lower, upper]
- Implements widening at loop headers for convergence
- Implements narrowing for improved precision
- Detects potential overflows when operations exceed type bounds

**Key Challenges:**
- Balancing precision vs. convergence with widening/narrowing
- Handling complex loop boundaries
- Managing different integer types and signedness

### Null Pointer Dereference Detection

**Technique:** Dataflow Analysis with Pointer Aliasing
- Tracks nullness states: NotNull, MaybeNull, Null
- Performs chaotic iteration with transfer functions
- Implements pointer analysis for alias tracking
- Flags dereferences on MaybeNull or Null pointers

**Key Challenges:**
- Handling pointer aliasing correctly
- Propagating nullness across assignments
- Managing interprocedural analysis (stretch goal)

## Setup Instructions

### Prerequisites

- Docker container with LLVM 14
- CMake 3.10+
- clang/opt tools
- Python 3 (for metrics calculation)

### Building the Project

#### Option 1: Inside Docker Container (Recommended)

```bash
# Enter the Docker container
docker exec -it great_williams zsh

# Navigate to project directory
cd /nullpointer

# Create build directory and compile
mkdir -p build
cd build
cmake ..
make -j4

# Verify build
ls -lh OverflowPass.so NullPtrPass.so
```

#### Option 2: Local Build

```bash
cd /path/to/nullpointer
mkdir -p build
cd build
cmake ..
make -j4
```

### Build Outputs

After successful build, you should see:
- `build/OverflowPass.so` - Integer overflow detection pass
- `build/NullPtrPass.so` - Null pointer detection pass (if built)

## Running the Analyses

### Quick Start - Interactive Menu

The easiest way to run tests:

```bash
cd /nullpointer
./test_menu.sh
```

Menu options:
- `1` - Run ALL tests (recommended first time)
- `2` - Run int_over_under_flow tests
- `3` - Run CWE_190 tests
- `4` - Run more_data_types tests
- `5` - Run a single test manually
- `6` - View test source code
- `7` - Calculate Precision & Recall metrics
- `8` - Clean all test outputs
- `9` - Rebuild OverflowPass

### Automated Test Script

Run all overflow tests with automatic metrics:

```bash
cd /nullpointer
./run_overflow_tests.sh
```

This will:
1. Run all tests in all test directories
2. Display results with color-coded output
3. Automatically calculate and show precision/recall metrics

### Manual Testing - Single File

```bash
# Navigate to test directory
cd /nullpointer/test/int_over_under_flow

# Compile C to LLVM IR
clang -emit-llvm -S -fno-discard-value-names -Xclang -disable-O0-optnone -c -o test01.ll test01.c

# Run mem2reg optimization
opt -mem2reg -S test01.ll -o test01.opt.ll

# Run overflow analysis
opt -load-pass-plugin ../../build/OverflowPass.so -passes=Overflow test01.opt.ll -disable-output

# View optimized IR (optional)
cat test01.opt.ll
```

### Using Makefiles

Each test directory has a Makefile:

```bash
cd /nullpointer/test/int_over_under_flow

# Run all tests in this directory
make all

# Run specific test
make test01

# View results
cat test01.out    # Overflow detection output
cat test01.err    # Dataflow analysis details

# Clean generated files
make clean
```

### Running Specific Test Suites

```bash
# Basic overflow/underflow tests
cd /nullpointer/test/int_over_under_flow && make all

# CWE-190 real-world examples
cd /nullpointer/test/CWE_190 && make all

# Complex data type tests
cd /nullpointer/test/more_data_types && make all

# Null pointer tests
cd /nullpointer/test/nullpointer && make all
```

## Interpreting Outputs

### Overflow Detection Output

Each test produces two output files:
- `.out` - Main overflow detection results
- `.err` - Detailed dataflow analysis information

#### Example: Overflow Detected

**Input (test01.c):**
```c
// Expect: fail
int main() {
  int p = __INT_MAX__;
  p++; // Error
  return 0;
}
```

**Output (test01.out):**
```
Running Overflow on main
Potential Overflow Instructions by Overflow:
  %inc = add nsw i32 2147483647, 1
```

**Interpretation:** Overflow DETECTED - The instruction `%inc = add nsw i32 2147483647, 1` is flagged as a potential overflow.

**Detailed Analysis (test01.err):**
```
Dataflow Analysis Results:
Instruction:   %inc = add nsw i32 2147483647, 1
In set:
Out set:
    [ %inc     |-> MaybeOverflow   ]
```

**Interpretation:** The dataflow domain shows `MaybeOverflow` for this instruction.

#### Example: No Overflow

**Output:**
```
Running Overflow on main
Potential Overflow Instructions by Overflow:

```

**Interpretation:** No overflow detected - the instruction list is empty.

### Test Result Classification

Tests are annotated with expected behavior:

```c
// Expect: fail   - Should detect overflow
// Expect: pass   - Should NOT detect overflow
// EXPECT: OVERFLOW - Alternative format
```

Specific error lines can be marked:
```c
int x = INT_MAX;
int y = x + 1;  // Error
```

### Precision & Recall Metrics

After running tests, metrics are calculated:

```
============================================================
INTEGER OVERFLOW DETECTION - PRECISION & RECALL
============================================================

Total Tests:        18
True Positives:     8    ← Correctly detected overflows
False Positives:    1    ← Incorrectly flagged as overflow
True Negatives:     7    ← Correctly identified no overflow
False Negatives:    2    ← Missed real overflows

------------------------------------------------------------
Precision:          88.89%    ← Of detections, how many were real
Recall:             80.00%    ← Of real overflows, how many detected
F1 Score:           84.21%    ← Harmonic mean of precision/recall
Accuracy:           83.33%    ← Overall correctness
------------------------------------------------------------
```

#### Understanding Metrics

- **Precision = TP / (TP + FP)**: Of all detected overflows, what percentage were real?
  - High precision = Few false alarms

- **Recall = TP / (TP + FN)**: Of all real overflows, what percentage did we catch?
  - High recall = We catch most bugs

- **F1 Score**: Harmonic mean balancing precision and recall
  - Good F1 > 85% indicates effective analysis

- **True Positive (TP)**: Correctly detected an overflow
- **False Positive (FP)**: Incorrectly flagged as overflow (false alarm)
- **True Negative (TN)**: Correctly identified no overflow
- **False Negative (FN)**: Missed a real overflow (dangerous!)

#### Calculate Metrics Manually

```bash
# Show detailed metrics with per-test breakdown
python3 calculate_metrics.py --verbose

# JSON output for tooling integration
python3 calculate_metrics.py --json

# Custom test directories
python3 calculate_metrics.py --test-dirs test/int_over_under_flow test/CWE_190
```

### Viewing Results Across Tests

```bash
# List all tests that detected overflows
cd /nullpointer/test/int_over_under_flow
grep -l "Potential Overflow" *.out

# View all detected overflows
grep -H "Potential Overflow" *.out

# Count detection rate
total=$(ls *.c | wc -l)
detected=$(grep -l "Potential Overflow" *.out 2>/dev/null | wc -l)
echo "Detected $detected out of $total tests"
```

## Test Suites

### 1. int_over_under_flow/ - Basic Tests

Simple, focused tests for overflow/underflow:
- `test01.c` - INT_MAX increment (should overflow)
- `test02.c` - Multiplication overflow
- `test03.c` - Underflow detection
- `test04.c` - Addition overflow
- `test05.c` - Subtraction underflow
- `test06.c` - Shift overflow

**Purpose:** Verify basic functionality and edge cases

### 2. CWE_190/ - Real-World Examples

Tests based on Common Weakness Enumeration 190:
- `test13.c` through `test18.c` - Real-world overflow patterns from CWE database

**Purpose:** Validate against known vulnerability patterns

### 3. more_data_types/ - Complex Tests

Tests with various data types and scenarios:
- Different integer types (short, long, unsigned)
- Complex expressions
- Mixed arithmetic operations

**Purpose:** Test robustness across data types

### 4. nullpointer/ - Null Pointer Tests

Tests for null pointer dereference detection:
- Basic null pointer tests (`test01.c` - `test18.c`)
- `aliasing/` - Pointer aliasing scenarios

**Purpose:** Validate null pointer analysis

## Reproducing Experiments

### Complete Workflow

```bash
# 1. Enter container
docker exec -it great_williams zsh
cd /nullpointer

# 2. Build the project
mkdir -p build && cd build
cmake ..
make -j4
cd ..

# 3. Run all overflow tests with metrics
./run_overflow_tests.sh

# 4. Review detailed results
cat test/int_over_under_flow/*.out
cat test/CWE_190/*.out
cat test/more_data_types/*.out

# 5. Calculate detailed metrics
python3 calculate_metrics.py --verbose

# 6. Clean and rebuild (if needed)
make clean
mkdir -p build && cd build && cmake .. && make -j4 && cd ..
```

### Validating Your Setup

After building, verify everything works:

```bash
# Check passes were built
ls -lh build/OverflowPass.so

# Run a single test
cd test/int_over_under_flow
make test01
cat test01.out

# Should see overflow detected:
# "Potential Overflow Instructions by Overflow:"
# "  %inc = add nsw i32 2147483647, 1"
```

## Performance Data

Performance metrics are tracked in `test/nullpointer/performance.csv`.

## Development Workflow

1. Make changes to analysis code (`src/OverflowAnalysis.cpp`, etc.)
2. Rebuild: `cd build && make`
3. Run tests: `./run_overflow_tests.sh`
4. Review metrics and investigate failures
5. Iterate on improvements

## Troubleshooting

### Build Issues

**"OverflowPass.so not found"**
```bash
# Make sure you're in the container
docker exec -it great_williams zsh
cd /nullpointer

# Clean and rebuild
rm -rf build
mkdir -p build && cd build
cmake ..
make -j4

# Verify
ls -la OverflowPass.so
```

**CMake errors**
```bash
# Ensure LLVM 14 is available
llvm-config --version  # Should show 14.x

# Clean and reconfigure
cd build
rm -rf *
cmake ..
make -j4
```

### Test Execution Issues

**Permission errors**
```bash
# Ensure scripts are executable
chmod +x run_overflow_tests.sh test_menu.sh test_with_timeout.sh
```

**Tests don't run**
```bash
# Check you're in correct directory
pwd  # Should be /nullpointer or similar

# Verify passes exist
ls -lh build/OverflowPass.so
```

**No output files**
```bash
# Run tests manually to see errors
cd test/int_over_under_flow
make test01
```

### Metrics Issues

**All zeros in metrics**
- Verify test files have ground truth annotations (`// Expect: fail` or `// Expect: pass`)
- Check that `.out` files exist and contain results
- Run tests first before calculating metrics

**Unexpected results**
```bash
# Review test annotations
grep -r "Expect:" test/

# Check actual detections
grep -r "Potential Overflow" test/**/*.out

# Use verbose mode
python3 calculate_metrics.py --verbose
```

## Documentation

- **QUICKSTART.md** - Quick start guide with TL;DR commands
- **TESTING_GUIDE.md** - Comprehensive testing documentation
- **METRICS_GUIDE.md** - Detailed precision/recall metrics guide
- **doc/project_proposal.md** - Original project proposal with goals

## References

- [CWE-190: Integer Overflow](https://cwe.mitre.org/data/definitions/190.html)
- [CWE-476: NULL Pointer Dereference](https://cwe.mitre.org/data/definitions/476.html)
- LLVM Pass Documentation
- Abstract Interpretation Theory

---

For quick testing: `./test_menu.sh` and select option 1!
