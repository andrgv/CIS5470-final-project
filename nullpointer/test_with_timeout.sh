#!/bin/bash

# Test individual files with timeout to identify problematic tests

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

TIMEOUT=10  # seconds per test

test_file() {
    local test=$1
    local dir=$2

    echo -n "Testing $test... "

    cd "/nullpointer/test/$dir"

    # Clean first
    rm -f ${test}.ll ${test}.opt.ll ${test}.out ${test}.err 2>/dev/null

    # Run with timeout
    timeout ${TIMEOUT}s make $test > /dev/null 2>&1

    local exit_code=$?

    if [ $exit_code -eq 124 ]; then
        echo -e "${RED}TIMEOUT (>${TIMEOUT}s) - INFINITE LOOP SUSPECTED${NC}"
        return 1
    elif [ $exit_code -ne 0 ]; then
        echo -e "${YELLOW}FAILED${NC}"
        return 1
    else
        echo -e "${GREEN}OK${NC}"
        return 0
    fi
}

echo "========================================="
echo "Testing CWE_190 with timeouts"
echo "========================================="
echo

for test in test13 test14 test15 test16 test17 test18; do
    test_file $test "CWE_190"
done

echo
echo "========================================="
echo "Testing more_data_types with timeouts"
echo "========================================="
echo

cd /nullpointer/test/more_data_types
for testfile in *.c; do
    test=$(basename $testfile .c)
    test_file $test "more_data_types"
done

echo
echo "Done! Check which tests timed out."
