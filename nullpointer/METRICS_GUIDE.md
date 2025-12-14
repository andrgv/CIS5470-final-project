# Precision & Recall Metrics Guide

## Overview

This guide explains how to use the precision and recall calculation system for evaluating the integer overflow detection analysis.

## Quick Start

### Run Tests and Calculate Metrics

The easiest way is to run all tests with automatic metrics calculation:

```bash
./run_overflow_tests.sh
```

This will:
1. Run all overflow tests in the three test directories
2. Automatically calculate and display precision/recall metrics
3. Show detailed results for each test

### Using the Interactive Menu

```bash
./test_menu.sh
```

Then select option **7** to calculate metrics on existing test results.

### Standalone Metrics Calculation

If you've already run tests and just want to see metrics:

```bash
python3 calculate_metrics.py --verbose
```

## Understanding the Metrics

### Key Metrics Explained

- **Precision**: Of all the overflows detected, what percentage were real overflows?
  - Formula: TP / (TP + FP)
  - High precision = Few false alarms

- **Recall**: Of all the real overflows, what percentage did we detect?
  - Formula: TP / (TP + FN)
  - High recall = We catch most overflows

- **F1 Score**: Harmonic mean of precision and recall
  - Formula: 2 × (Precision × Recall) / (Precision + Recall)
  - Balances precision and recall

- **Accuracy**: Overall correctness
  - Formula: (TP + TN) / Total
  - Less useful for imbalanced datasets

### Confusion Matrix Terms

- **True Positive (TP)**: Correctly detected an overflow that exists
- **False Positive (FP)**: Incorrectly detected an overflow that doesn't exist
- **True Negative (TN)**: Correctly identified no overflow
- **False Negative (FN)**: Missed an overflow that exists

## Annotating Test Files

The metrics system uses comments in your C test files as ground truth.

### Annotation Format

**Method 1: Test-level annotation**
```c
// Expect: fail
int main() {
    int x = INT_MAX;
    x++; // This should overflow
    return 0;
}
```

**Method 2: Alternative format**
```c
// EXPECT: OVERFLOW
int main() {
    int x = INT_MAX;
    x++; // This should overflow
    return 0;
}
```

**For tests that should pass:**
```c
// Expect: pass
int main() {
    int x = 100;
    int y = x + 1; // No overflow
    return 0;
}
```

### Marking Specific Error Lines

Use `// Error` comments to mark specific lines where overflows occur:

```c
// Expect: fail
int main() {
    int a = INT_MAX;
    int b = a + 1;  // Error

    int c = 100;
    int d = c * 2;  // Error

    return 0;
}
```

This helps with detailed analysis of false negatives.

## Example Output

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

Detailed Results by Test:
------------------------------------------------------------

✓ TRUE POSITIVES (Correctly detected overflows):
  test01: 1 overflow(s) detected
  test02: 2 overflow(s) detected
  ...

✗ FALSE NEGATIVES (Missed overflows):
  test05: Expected overflow but none detected
    Expected errors at lines: [6]
  ...
```

## Command Line Options

### Show Detailed Results

```bash
python3 calculate_metrics.py --verbose
```

Shows per-test breakdown with TP/FP/TN/FN categories.

### JSON Output

```bash
python3 calculate_metrics.py --json
```

Outputs metrics in JSON format for integration with other tools.

### Custom Test Directories

```bash
python3 calculate_metrics.py --test-dirs test/int_over_under_flow test/CWE_190
```

Calculate metrics for specific test directories only.

## Interpreting Results

### Good Results
- **High Precision (>90%)**: Few false alarms, users can trust detections
- **High Recall (>80%)**: Catching most real overflows
- **High F1 (>85%)**: Good balance between precision and recall

### Common Issues

**Low Precision**
- Too many false positives
- Analysis is too conservative
- Consider refining widening/narrowing strategies

**Low Recall**
- Missing real overflows
- Analysis may be too optimistic
- May need better interval tracking or loop handling

**Low Both**
- Analysis needs significant improvement
- Check transfer functions and domain implementation


## Integration with Development Workflow

1. Make changes to your overflow analysis
2. Rebuild: `cd build && make`
3. Run tests: `./run_overflow_tests.sh`
4. Review metrics automatically displayed
5. Investigate failures using detailed output
6. Iterate on improvements

## See Also

- `TESTING_GUIDE.md`: General testing documentation
- `doc/project_proposal.md`: Project overview and goals
