#!/bin/bash

# Interactive Test Menu for Overflow Analysis

GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

show_menu() {
    clear
    echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║   Overflow Analysis Test Menu         ║${NC}"
    echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
    echo
    echo "1) Run ALL overflow tests"
    echo "2) Run int_over_under_flow tests"
    echo "3) Run CWE_190 tests"
    echo "4) Run more_data_types tests"
    echo "5) Run single test (manual)"
    echo "6) View test file"
    echo "7) Calculate Precision & Recall"
    echo "8) Clean all test outputs"
    echo "9) Rebuild OverflowPass"
    echo "h) Help / Guide"
    echo "0) Exit"
    echo
    echo -n "Select option: "
}

run_all_tests() {
    echo -e "${GREEN}Running all overflow tests...${NC}"
    echo
    cd /nullpointer
    ./run_overflow_tests.sh
    echo
    read -p "Press Enter to continue..."
}

run_test_dir() {
    local dir=$1
    local name=$2
    local start_dir=$(pwd)

    echo -e "${GREEN}Running ${name}...${NC}"
    echo
    cd "/nullpointer/test/${dir}"
    make all 2>&1
    echo
    echo -e "${YELLOW}Results summary:${NC}"
    for f in *.out; do
        if [ -f "$f" ]; then
            echo "--- $f ---"
            grep -E "(Running|Potential)" "$f" || echo "No overflows detected"
        fi
    done
    echo
    cd "$start_dir"
    read -p "Press Enter to continue..."
}

run_single_test() {
    echo -e "${YELLOW}Available test directories:${NC}"
    echo "1) int_over_under_flow"
    echo "2) CWE_190"
    echo "3) more_data_types"
    echo -n "Select directory: "
    read dir_choice

    case $dir_choice in
        1) test_dir="int_over_under_flow" ;;
        2) test_dir="CWE_190" ;;
        3) test_dir="more_data_types" ;;
        *) echo "Invalid choice"; read -p "Press Enter..."; return ;;
    esac

    cd "/nullpointer/test/${test_dir}"
    echo
    echo -e "${YELLOW}Available tests:${NC}"
    ls *.c 2>/dev/null | nl
    echo
    echo -n "Enter test name (e.g., test01): "
    read testname

    if [ -f "${testname}.c" ]; then
        echo
        echo -e "${GREEN}Running ${testname}...${NC}"
        make ${testname}
        echo
        echo -e "${YELLOW}=== Results (${testname}.out) ===${NC}"
        cat ${testname}.out
        echo
        echo -e "${YELLOW}=== Dataflow Details (${testname}.err) ===${NC}"
        cat ${testname}.err | head -30
    else
        echo "Test file not found: ${testname}.c"
    fi
    echo
    read -p "Press Enter to continue..."
}

view_test_file() {
    echo -e "${YELLOW}Available test directories:${NC}"
    echo "1) int_over_under_flow"
    echo "2) CWE_190"
    echo "3) more_data_types"
    echo -n "Select directory: "
    read dir_choice

    case $dir_choice in
        1) test_dir="int_over_under_flow" ;;
        2) test_dir="CWE_190" ;;
        3) test_dir="more_data_types" ;;
        *) echo "Invalid choice"; read -p "Press Enter..."; return ;;
    esac

    cd "/nullpointer/test/${test_dir}"
    echo
    echo -e "${YELLOW}Available tests:${NC}"
    ls *.c 2>/dev/null | nl
    echo
    echo -n "Enter test name (e.g., test01): "
    read testname

    if [ -f "${testname}.c" ]; then
        echo
        echo -e "${GREEN}=== ${testname}.c ===${NC}"
        cat ${testname}.c
    else
        echo "Test file not found: ${testname}.c"
    fi
    echo
    read -p "Press Enter to continue..."
}

calculate_metrics() {
    clear
    echo -e "${BLUE}=== Precision & Recall Calculation ===${NC}"
    echo
    cd /nullpointer
    if [ -f "calculate_metrics.py" ]; then
        python3 calculate_metrics.py --verbose
    else
        echo -e "${RED}Error: calculate_metrics.py not found!${NC}"
    fi
    echo
    read -p "Press Enter to continue..."
}

clean_all() {
    echo -e "${YELLOW}Cleaning all test outputs...${NC}"
    cd /nullpointer/test/int_over_under_flow && make clean 2>/dev/null
    cd /nullpointer/test/CWE_190 && make clean 2>/dev/null
    cd /nullpointer/test/more_data_types && make clean 2>/dev/null
    echo -e "${GREEN}Done!${NC}"
    read -p "Press Enter to continue..."
}

rebuild() {
    echo -e "${YELLOW}Rebuilding OverflowPass...${NC}"
    cd /nullpointer/build
    make OverflowPass
    echo
    if [ -f "OverflowPass.so" ]; then
        echo -e "${GREEN}✓ Build successful!${NC}"
    else
        echo -e "${RED}✗ Build failed!${NC}"
    fi
    echo
    read -p "Press Enter to continue..."
}

show_help() {
    clear
    echo -e "${BLUE}=== Overflow Analysis Help ===${NC}"
    echo
    echo "This menu helps you test the overflow analysis implementation."
    echo
    echo "Quick Start:"
    echo "  1. Choose option 1 to run all tests automatically"
    echo "  2. View results and look for 'Potential Overflow Instructions'"
    echo
    echo "Manual Testing:"
    echo "  1. Choose option 5 to run a specific test"
    echo "  2. Choose option 6 to view test source code"
    echo
    echo "Test Files:"
    echo "  - .c files: Original C source code"
    echo "  - .ll files: LLVM IR (generated)"
    echo "  - .opt.ll: Optimized LLVM IR (generated)"
    echo "  - .out: Overflow detection results"
    echo "  - .err: Detailed dataflow analysis"
    echo
    echo "For full documentation, see: TESTING_GUIDE.md"
    echo
    read -p "Press Enter to continue..."
}

# Main loop
while true; do
    show_menu
    read choice

    case $choice in
        1) run_all_tests ;;
        2) run_test_dir "int_over_under_flow" "int_over_under_flow tests" ;;
        3) run_test_dir "CWE_190" "CWE_190 tests" ;;
        4) run_test_dir "more_data_types" "more_data_types tests" ;;
        5) run_single_test ;;
        6) view_test_file ;;
        7) calculate_metrics ;;
        8) clean_all ;;
        9) rebuild ;;
        h|H) show_help ;;
        0) echo "Goodbye!"; exit 0 ;;
        *) echo "Invalid option"; read -p "Press Enter..."; ;;
    esac
done
