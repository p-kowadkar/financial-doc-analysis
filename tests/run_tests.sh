#!/bin/bash

# Comprehensive Test Runner for C++ Financial Document Analysis System
# This script builds and runs all tests in the correct order

echo "=========================================="
echo "C++ Financial Document Analysis System"
echo "Comprehensive Test Suite"
echo "=========================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test configuration
TEST_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$TEST_DIR/../src"
BUILD_DIR="$TEST_DIR/build"
RESULTS_DIR="$TEST_DIR/test_results"
DATA_DIR="$TEST_DIR/test_data"

# Create necessary directories
mkdir -p "$BUILD_DIR"
mkdir -p "$RESULTS_DIR"
mkdir -p "$DATA_DIR"

# Compilation flags
CXX_FLAGS="-std=c++17 -Wall -Wextra -O2"
INCLUDE_FLAGS="-I$TEST_DIR -I$SRC_DIR"

# Test results tracking
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Function to print colored output
print_status() {
    local status=$1
    local message=$2
    case $status in
        "INFO")
            echo -e "${BLUE}[INFO]${NC} $message"
            ;;
        "SUCCESS")
            echo -e "${GREEN}[SUCCESS]${NC} $message"
            ;;
        "WARNING")
            echo -e "${YELLOW}[WARNING]${NC} $message"
            ;;
        "ERROR")
            echo -e "${RED}[ERROR]${NC} $message"
            ;;
    esac
}

# Function to run a test
run_test() {
    local test_name=$1
    local test_executable=$2
    local test_category=$3
    
    print_status "INFO" "Running $test_category: $test_name"
    
    if [ ! -f "$test_executable" ]; then
        print_status "ERROR" "Test executable not found: $test_executable"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        return 1
    fi
    
    # Run the test and capture output
    local output_file="$RESULTS_DIR/${test_name}_output.log"
    local start_time=$(date +%s.%N)
    
    if "$test_executable" > "$output_file" 2>&1; then
        local end_time=$(date +%s.%N)
        local duration=$(echo "$end_time - $start_time" | bc -l)
        print_status "SUCCESS" "$test_name completed in ${duration}s"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        return 0
    else
        local end_time=$(date +%s.%N)
        local duration=$(echo "$end_time - $start_time" | bc -l)
        print_status "ERROR" "$test_name failed after ${duration}s"
        print_status "ERROR" "Check log: $output_file"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        return 1
    fi
}

# Function to compile a test
compile_test() {
    local test_source=$1
    local test_executable=$2
    local additional_flags=$3
    
    print_status "INFO" "Compiling $(basename "$test_source")"
    
    if g++ $CXX_FLAGS $INCLUDE_FLAGS $additional_flags "$test_source" -o "$test_executable" 2>"$BUILD_DIR/compile_$(basename "$test_executable").log"; then
        print_status "SUCCESS" "Compiled $(basename "$test_executable")"
        return 0
    else
        print_status "ERROR" "Failed to compile $(basename "$test_source")"
        print_status "ERROR" "Check log: $BUILD_DIR/compile_$(basename "$test_executable").log"
        return 1
    fi
}

# Main test execution
main() {
    print_status "INFO" "Starting comprehensive test suite"
    print_status "INFO" "Test directory: $TEST_DIR"
    print_status "INFO" "Source directory: $SRC_DIR"
    print_status "INFO" "Build directory: $BUILD_DIR"
    print_status "INFO" "Results directory: $RESULTS_DIR"
    
    # Check if source files exist
    if [ ! -d "$SRC_DIR" ]; then
        print_status "ERROR" "Source directory not found: $SRC_DIR"
        exit 1
    fi
    
    echo ""
    print_status "INFO" "Phase 1: Compiling Unit Tests"
    echo "----------------------------------------"
    
    # Compile unit tests
    UNIT_TESTS=(
        "unit/test_document_parser.cpp:test_document_parser"
        "unit/test_tfidf_embedding.cpp:test_tfidf_embedding"
    )
    
    for test_info in "${UNIT_TESTS[@]}"; do
        IFS=':' read -r test_source test_name <<< "$test_info"
        test_executable="$BUILD_DIR/$test_name"
        
        if compile_test "$TEST_DIR/$test_source" "$test_executable" ""; then
            TOTAL_TESTS=$((TOTAL_TESTS + 1))
        fi
    done
    
    echo ""
    print_status "INFO" "Phase 2: Compiling Integration Tests"
    echo "----------------------------------------"
    
    # Compile integration tests
    INTEGRATION_TESTS=(
        "integration/test_system_integration.cpp:test_system_integration"
    )
    
    for test_info in "${INTEGRATION_TESTS[@]}"; do
        IFS=':' read -r test_source test_name <<< "$test_info"
        test_executable="$BUILD_DIR/$test_name"
        
        if compile_test "$TEST_DIR/$test_source" "$test_executable" ""; then
            TOTAL_TESTS=$((TOTAL_TESTS + 1))
        fi
    done
    
    echo ""
    print_status "INFO" "Phase 3: Running Unit Tests"
    echo "----------------------------------------"
    
    # Run unit tests
    for test_info in "${UNIT_TESTS[@]}"; do
        IFS=':' read -r test_source test_name <<< "$test_info"
        test_executable="$BUILD_DIR/$test_name"
        run_test "$test_name" "$test_executable" "Unit Test"
    done
    
    echo ""
    print_status "INFO" "Phase 4: Running Integration Tests"
    echo "----------------------------------------"
    
    # Run integration tests
    for test_info in "${INTEGRATION_TESTS[@]}"; do
        IFS=':' read -r test_source test_name <<< "$test_info"
        test_executable="$BUILD_DIR/$test_name"
        run_test "$test_name" "$test_executable" "Integration Test"
    done
    
    echo ""
    print_status "INFO" "Phase 5: Performance Benchmarks"
    echo "----------------------------------------"
    
    # Run performance tests if available
    if [ -f "$BUILD_DIR/test_performance" ]; then
        run_test "performance_benchmark" "$BUILD_DIR/test_performance" "Performance Test"
    else
        print_status "WARNING" "Performance tests not available"
    fi
    
    echo ""
    print_status "INFO" "Phase 6: Generating Test Report"
    echo "----------------------------------------"
    
    # Generate comprehensive test report
    local report_file="$RESULTS_DIR/test_report.md"
    generate_test_report "$report_file"
    
    echo ""
    echo "=========================================="
    echo "TEST SUITE SUMMARY"
    echo "=========================================="
    echo "Total Tests: $TOTAL_TESTS"
    echo "Passed: $PASSED_TESTS"
    echo "Failed: $FAILED_TESTS"
    
    if [ $FAILED_TESTS -eq 0 ]; then
        print_status "SUCCESS" "ALL TESTS PASSED! 🎉"
        echo "Success Rate: 100%"
    else
        print_status "ERROR" "$FAILED_TESTS TESTS FAILED"
        local success_rate=$(echo "scale=1; $PASSED_TESTS * 100 / $TOTAL_TESTS" | bc -l)
        echo "Success Rate: ${success_rate}%"
    fi
    
    echo ""
    print_status "INFO" "Test results saved to: $RESULTS_DIR"
    print_status "INFO" "Detailed report: $report_file"
    
    # Return appropriate exit code
    if [ $FAILED_TESTS -eq 0 ]; then
        exit 0
    else
        exit 1
    fi
}

# Function to generate test report
generate_test_report() {
    local report_file=$1
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    
    cat > "$report_file" << EOF
# C++ Financial Document Analysis System - Test Report

**Generated:** $timestamp  
**Test Suite Version:** 1.0  
**Platform:** $(uname -s) $(uname -m)  
**Compiler:** $(g++ --version | head -n1)

## Summary

- **Total Tests:** $TOTAL_TESTS
- **Passed:** $PASSED_TESTS
- **Failed:** $FAILED_TESTS
- **Success Rate:** $(echo "scale=1; $PASSED_TESTS * 100 / $TOTAL_TESTS" | bc -l)%

## Test Categories

### Unit Tests
- Document Parser Tests
- TF-IDF Embedding Tests

### Integration Tests
- End-to-End System Integration
- Query Engine Integration
- Financial Analyzer Integration
- RAG Engine Integration

### Performance Tests
- System Performance Benchmarks
- Memory Usage Analysis
- Scalability Tests

## Detailed Results

EOF

    # Add detailed results from individual test outputs
    for log_file in "$RESULTS_DIR"/*_output.log; do
        if [ -f "$log_file" ]; then
            local test_name=$(basename "$log_file" _output.log)
            echo "### $test_name" >> "$report_file"
            echo '```' >> "$report_file"
            tail -n 20 "$log_file" >> "$report_file"
            echo '```' >> "$report_file"
            echo "" >> "$report_file"
        fi
    done
    
    cat >> "$report_file" << EOF

## System Information

- **OS:** $(uname -a)
- **CPU:** $(grep "model name" /proc/cpuinfo | head -n1 | cut -d: -f2 | xargs)
- **Memory:** $(free -h | grep "Mem:" | awk '{print $2}')
- **Disk Space:** $(df -h . | tail -n1 | awk '{print $4}') available

## Test Environment

- **Source Directory:** $SRC_DIR
- **Test Directory:** $TEST_DIR
- **Build Directory:** $BUILD_DIR
- **Results Directory:** $RESULTS_DIR

## Recommendations

EOF

    if [ $FAILED_TESTS -eq 0 ]; then
        echo "✅ All tests passed successfully. The system is ready for production deployment." >> "$report_file"
    else
        echo "❌ Some tests failed. Please review the failed tests and fix issues before deployment." >> "$report_file"
        echo "" >> "$report_file"
        echo "**Failed Tests:**" >> "$report_file"
        for log_file in "$RESULTS_DIR"/*_output.log; do
            if [ -f "$log_file" ] && grep -q "FAIL\|ERROR" "$log_file"; then
                local test_name=$(basename "$log_file" _output.log)
                echo "- $test_name" >> "$report_file"
            fi
        done
    fi
    
    print_status "SUCCESS" "Test report generated: $report_file"
}

# Check dependencies
check_dependencies() {
    local missing_deps=()
    
    if ! command -v g++ &> /dev/null; then
        missing_deps+=("g++")
    fi
    
    if ! command -v bc &> /dev/null; then
        missing_deps+=("bc")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_status "ERROR" "Missing dependencies: ${missing_deps[*]}"
        print_status "INFO" "Please install missing dependencies and try again"
        exit 1
    fi
}

# Script entry point
if [ "${BASH_SOURCE[0]}" == "${0}" ]; then
    check_dependencies
    main "$@"
fi

