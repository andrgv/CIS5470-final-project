# CIS 5470 Final Project: Integer Overflow & Null Pointer Detection

**Team Members:** Amanda and Andrea

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
│   └── nullpointer/           # Null pointer tests (20 tests)
│       └── aliasing/          # Pointer aliasing tests (10 tests)
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

## Setup Instructions

### Prerequisites

- Docker container with LLVM 14
- CMake 3.10+
- clang/opt tools
- Python 3 (for metrics calculation)

### Building the Project

#### Build

```bash
# Open the project in the dev container

# Create build directory and compile
mkdir build
cd build
cmake ..
make

# Verify build
ls -lh OverflowPass.so NullPtrPass.so
```

### Build Outputs

After successful build, you should see:
- `build/OverflowPass.so` - Integer overflow detection pass
- `build/NullPtrPass.so` - Null pointer detection pass

## Running the Analyses

### Automated Test Script

Run all overflow tests with automatic metrics:

```bash
./run_overflow_tests.sh
```

This will:
1. Run all tests in all test directories
2. Display results with color-coded output
3. Automatically calculate and show precision/recall metrics

To run null pointer dereference non-aliasing tests:
```bash
cd /test/nullpointer
./run_tests.sh
```

To run null pointer dereference aliasing tests:
```bash
cd /test/nullpointer/aliasing
./run_tests.sh
```

The script displays a table with the expected result, the analysis-reported
result, and whether the analysis's result is correct. It also displays the
precision, recall, and F1 metrics and the number of true/false positives
and true/false negatives.

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

The interpretation for null pointer dereference output is similar. The end of the *.out files contains instructions that are identified to contain null pointer dereferences, and test cases are also annotated with the expected result.


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
- Basic null pointer tests (`test01.c` - `test20.c`)
- `aliasing/` - Pointer aliasing scenarios

**Purpose:** Validate null pointer analysis


## References

- [CWE-190: Integer Overflow](https://cwe.mitre.org/data/definitions/190.html)
- LLVM Pass Documentation
- CIS 5470 labs 6 and 7 and slides
