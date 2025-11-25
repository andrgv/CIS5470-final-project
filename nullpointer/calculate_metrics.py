#!/usr/bin/env python3
"""
Precision and Recall Calculator for Overflow Detection
Parses test files for ground truth and compares with analysis results.
"""

import os
import re
import sys
import argparse
from pathlib import Path
from typing import Dict, List, Set, Tuple
from dataclasses import dataclass


@dataclass
class TestResult:
    """Stores ground truth and detected results for a single test."""
    test_name: str
    test_file: str
    should_overflow: bool  # Ground truth: should this test have overflows?
    error_lines: Set[int]  # Line numbers marked with // Error
    detected_overflows: Set[str]  # Set of detected overflow instructions
    has_detection: bool  # Did the analysis detect any overflow?


class GroundTruthParser:
    """Parses C test files to extract ground truth annotations."""

    @staticmethod
    def parse_test_file(filepath: str) -> Tuple[bool, Set[int]]:
        """
        Parse a test file for ground truth annotations.

        Returns:
            (should_overflow, error_lines)
            - should_overflow: True if test expects overflow detection
            - error_lines: Set of line numbers with // Error comments
        """
        should_overflow = False
        error_lines = set()

        with open(filepath, 'r') as f:
            for line_num, line in enumerate(f, 1):
                line_lower = line.lower()

                # Check for expect annotations
                if '// expect:' in line_lower or '// expect :' in line_lower:
                    if 'fail' in line_lower or 'overflow' in line_lower:
                        should_overflow = True
                    elif 'pass' in line_lower:
                        should_overflow = False

                # Check for Error markers
                if '// error' in line_lower:
                    error_lines.add(line_num)

        return should_overflow, error_lines


class ResultParser:
    """Parses .out files from overflow analysis."""

    @staticmethod
    def parse_output_file(filepath: str) -> Set[str]:
        """
        Parse a .out file to extract detected overflow instructions.

        Returns:
            Set of detected overflow instruction strings
        """
        detected = set()

        if not os.path.exists(filepath):
            return detected

        with open(filepath, 'r') as f:
            content = f.read()

            # Look for overflow detections
            if 'Potential Overflow Instructions' in content:
                # Extract the instruction lines after the header
                lines = content.split('\n')
                in_overflow_section = False

                for line in lines:
                    if 'Potential Overflow Instructions' in line:
                        in_overflow_section = True
                        continue

                    if in_overflow_section:
                        line = line.strip()
                        if line and not line.startswith('Running'):
                            detected.add(line)

        return detected


class MetricsCalculator:
    """Calculates precision, recall, and other metrics."""

    def __init__(self):
        self.results: List[TestResult] = []

    def add_result(self, result: TestResult):
        """Add a test result."""
        self.results.append(result)

    def calculate_metrics(self) -> Dict:
        """
        Calculate precision, recall, F1, and accuracy.

        For overflow detection:
        - TP: Test should overflow AND detection found overflow
        - FP: Test should NOT overflow BUT detection found overflow
        - TN: Test should NOT overflow AND detection found no overflow
        - FN: Test should overflow BUT detection found no overflow
        """
        tp = 0  # True Positives
        fp = 0  # False Positives
        tn = 0  # True Negatives
        fn = 0  # False Negatives

        for result in self.results:
            if result.should_overflow and result.has_detection:
                tp += 1
            elif not result.should_overflow and result.has_detection:
                fp += 1
            elif not result.should_overflow and not result.has_detection:
                tn += 1
            elif result.should_overflow and not result.has_detection:
                fn += 1

        total = tp + fp + tn + fn

        # Calculate metrics
        precision = tp / (tp + fp) if (tp + fp) > 0 else 0.0
        recall = tp / (tp + fn) if (tp + fn) > 0 else 0.0
        f1 = 2 * (precision * recall) / (precision + recall) if (precision + recall) > 0 else 0.0
        accuracy = (tp + tn) / total if total > 0 else 0.0

        return {
            'tp': tp,
            'fp': fp,
            'tn': tn,
            'fn': fn,
            'total': total,
            'precision': precision,
            'recall': recall,
            'f1': f1,
            'accuracy': accuracy
        }

    def print_summary(self, metrics: Dict, verbose: bool = False):
        """Print a formatted summary of metrics."""
        print("\n" + "=" * 60)
        print("INTEGER OVERFLOW DETECTION - PRECISION & RECALL")
        print("=" * 60)
        print()
        print(f"Total Tests:        {metrics['total']}")
        print(f"True Positives:     {metrics['tp']}")
        print(f"False Positives:    {metrics['fp']}")
        print(f"True Negatives:     {metrics['tn']}")
        print(f"False Negatives:    {metrics['fn']}")
        print()
        print("-" * 60)
        print(f"Precision:          {metrics['precision']:.2%}")
        print(f"Recall:             {metrics['recall']:.2%}")
        print(f"F1 Score:           {metrics['f1']:.2%}")
        print(f"Accuracy:           {metrics['accuracy']:.2%}")
        print("-" * 60)
        print()

        if verbose:
            self.print_detailed_results()

    def print_detailed_results(self):
        """Print detailed per-test results."""
        print("\nDetailed Results by Test:")
        print("-" * 60)

        # Group by category
        tp_tests = [r for r in self.results if r.should_overflow and r.has_detection]
        fp_tests = [r for r in self.results if not r.should_overflow and r.has_detection]
        tn_tests = [r for r in self.results if not r.should_overflow and not r.has_detection]
        fn_tests = [r for r in self.results if r.should_overflow and not r.has_detection]

        if tp_tests:
            print("\n✓ TRUE POSITIVES (Correctly detected overflows):")
            for r in tp_tests:
                print(f"  {r.test_name}: {len(r.detected_overflows)} overflow(s) detected")

        if tn_tests:
            print("\n✓ TRUE NEGATIVES (Correctly identified no overflow):")
            for r in tn_tests:
                print(f"  {r.test_name}: No overflow detected (correct)")

        if fp_tests:
            print("\n✗ FALSE POSITIVES (Incorrectly detected overflows):")
            for r in fp_tests:
                print(f"  {r.test_name}: {len(r.detected_overflows)} overflow(s) detected but none expected")
                for overflow in r.detected_overflows:
                    print(f"    - {overflow}")

        if fn_tests:
            print("\n✗ FALSE NEGATIVES (Missed overflows):")
            for r in fn_tests:
                print(f"  {r.test_name}: Expected overflow but none detected")
                if r.error_lines:
                    print(f"    Expected errors at lines: {sorted(r.error_lines)}")

        print()


def process_test_directory(test_dir: str, gt_parser: GroundTruthParser,
                          result_parser: ResultParser) -> List[TestResult]:
    """Process all tests in a directory."""
    results = []
    test_path = Path(test_dir)

    if not test_path.exists():
        print(f"Warning: Directory {test_dir} not found")
        return results

    # Find all .c test files
    c_files = sorted(test_path.glob("*.c"))

    for c_file in c_files:
        test_name = c_file.stem

        # Parse ground truth
        should_overflow, error_lines = gt_parser.parse_test_file(str(c_file))

        # Parse results
        out_file = test_path / f"{test_name}.out"
        detected_overflows = result_parser.parse_output_file(str(out_file))
        has_detection = len(detected_overflows) > 0

        result = TestResult(
            test_name=test_name,
            test_file=str(c_file),
            should_overflow=should_overflow,
            error_lines=error_lines,
            detected_overflows=detected_overflows,
            has_detection=has_detection
        )
        results.append(result)

    return results


def main():
    parser = argparse.ArgumentParser(
        description='Calculate precision and recall for overflow detection'
    )
    parser.add_argument(
        '--test-dirs',
        nargs='+',
        default=[
            'test/int_over_under_flow',
            'test/CWE_190',
            'test/more_data_types'
        ],
        help='Test directories to analyze'
    )
    parser.add_argument(
        '--verbose', '-v',
        action='store_true',
        help='Show detailed per-test results'
    )
    parser.add_argument(
        '--json',
        action='store_true',
        help='Output results in JSON format'
    )

    args = parser.parse_args()

    # Initialize parsers and calculator
    gt_parser = GroundTruthParser()
    result_parser = ResultParser()
    calculator = MetricsCalculator()

    # Process each test directory
    for test_dir in args.test_dirs:
        results = process_test_directory(test_dir, gt_parser, result_parser)
        for result in results:
            calculator.add_result(result)

    # Calculate and display metrics
    metrics = calculator.calculate_metrics()

    if args.json:
        import json
        print(json.dumps(metrics, indent=2))
    else:
        calculator.print_summary(metrics, verbose=args.verbose)

    # Return exit code based on whether we have good metrics
    # (non-zero precision and recall)
    if metrics['precision'] > 0 and metrics['recall'] > 0:
        return 0
    else:
        return 1


if __name__ == '__main__':
    sys.exit(main())
