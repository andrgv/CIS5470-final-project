#!/bin/bash

# Overflow Analysis Test Runner
# This script runs all overflow tests and summarizes results

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Overflow Analysis Test Runner${NC}"
echo -e "${BLUE}========================================${NC}"
echo

# Function to run tests in a directory
run_test_dir() {
    local dir=$1
    local name=$2
    local start_dir=$(pwd)

    echo -e "${YELLOW}Running tests in: ${name}${NC}"
    echo "Directory: $dir"
    echo

    cd "$dir"

    # Clean previous results
    make clean 2>/dev/null || true

    # Run all tests
    make all 2>&1 | grep -E "(Running|Potential|Error|Warning)" || true

    echo
    echo -e "${GREEN}Results for ${name}:${NC}"
    echo "----------------------------------------"

    # Count and display results
    local total=$(ls *.c 2>/dev/null | wc -l)
    local detected=0

    for outfile in *.out; do
        if [ -f "$outfile" ]; then
            local testname=$(basename "$outfile" .out)
            echo -e "${BLUE}Test: ${testname}${NC}"

            # Show if overflow was detected
            if grep -q "Potential Overflow Instructions" "$outfile"; then
                detected=$((detected + 1))
                echo -e "  ${GREEN}✓ Overflow detected${NC}"
                grep -A 10 "Potential Overflow Instructions" "$outfile" | sed 's/^/    /'
            else
                echo -e "  ${YELLOW}○ No overflow detected${NC}"
            fi
            echo
        fi
    done

    echo -e "${GREEN}Summary: ${detected}/${total} tests detected overflows${NC}"
    echo

    # Return to starting directory
    cd "$start_dir"
}

# Check if build exists
if [ ! -f "build/OverflowPass.so" ]; then
    echo -e "${RED}Error: OverflowPass.so not found!${NC}"
    echo "Please build the project first:"
    echo "  cd /nullpointer"
    echo "  mkdir -p build && cd build"
    echo "  cmake .."
    echo "  make"
    exit 1
fi

# Run tests in each directory
echo -e "${BLUE}Starting test suite...${NC}"
echo

# Test 1: int_over_under_flow
if [ -d "test/int_over_under_flow" ]; then
    run_test_dir "test/int_over_under_flow" "Integer Overflow/Underflow Tests"
else
    echo -e "${YELLOW}Warning: test/int_over_under_flow not found${NC}"
fi

# Test 2: CWE_190
if [ -d "test/CWE_190" ]; then
    run_test_dir "test/CWE_190" "CWE-190 Tests"
else
    echo -e "${YELLOW}Warning: test/CWE_190 not found${NC}"
fi

# Test 3: more_data_types
if [ -d "test/more_data_types" ]; then
    run_test_dir "test/more_data_types" "More Data Types Tests"
else
    echo -e "${YELLOW}Warning: test/more_data_types not found${NC}"
fi

echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}All tests complete!${NC}"
echo -e "${BLUE}========================================${NC}"

# Calculate precision and recall metrics
if [ -f "calculate_metrics.py" ]; then
    echo
    echo -e "${BLUE}Calculating Precision & Recall Metrics...${NC}"
    echo
    python3 calculate_metrics.py --verbose
else
    echo -e "${YELLOW}Warning: calculate_metrics.py not found${NC}"
fi
