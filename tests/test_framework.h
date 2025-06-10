#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <map>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <algorithm>

// Simple test framework for C++ without external dependencies
class TestFramework {
private:
    struct TestResult {
        std::string name;
        bool passed;
        std::string message;
        double executionTimeMs;
        std::string category;
    };
    
    std::vector<TestResult> results;
    std::string currentCategory;
    int totalTests;
    int passedTests;
    int failedTests;
    
public:
    TestFramework() : totalTests(0), passedTests(0), failedTests(0) {}
    
    void setCategory(const std::string& category) {
        currentCategory = category;
    }
    
    void runTest(const std::string& testName, std::function<bool()> testFunc) {
        auto start = std::chrono::high_resolution_clock::now();
        
        bool result = false;
        std::string message = "";
        
        try {
            result = testFunc();
            if (result) {
                message = "PASSED";
                passedTests++;
            } else {
                message = "FAILED";
                failedTests++;
            }
        } catch (const std::exception& e) {
            result = false;
            message = "EXCEPTION: " + std::string(e.what());
            failedTests++;
        } catch (...) {
            result = false;
            message = "UNKNOWN EXCEPTION";
            failedTests++;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double timeMs = duration.count() / 1000.0;
        
        TestResult testResult;
        testResult.name = testName;
        testResult.passed = result;
        testResult.message = message;
        testResult.executionTimeMs = timeMs;
        testResult.category = currentCategory;
        
        results.push_back(testResult);
        totalTests++;
        
        // Print immediate result
        std::cout << "[" << (result ? "PASS" : "FAIL") << "] " 
                  << currentCategory << "::" << testName 
                  << " (" << std::fixed << std::setprecision(2) << timeMs << "ms)" 
                  << std::endl;
        
        if (!result) {
            std::cout << "  Error: " << message << std::endl;
        }
    }
    
    void printSummary() {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "TEST SUMMARY" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        
        // Group by category
        std::map<std::string, std::vector<TestResult*>> categoryResults;
        for (auto& result : results) {
            categoryResults[result.category].push_back(&result);
        }
        
        for (const auto& category : categoryResults) {
            std::cout << "\n" << category.first << ":" << std::endl;
            
            int categoryPassed = 0;
            int categoryTotal = category.second.size();
            double totalTime = 0;
            
            for (const auto& result : category.second) {
                std::cout << "  " << (result->passed ? "✓" : "✗") 
                          << " " << result->name 
                          << " (" << std::fixed << std::setprecision(2) 
                          << result->executionTimeMs << "ms)" << std::endl;
                
                if (result->passed) categoryPassed++;
                totalTime += result->executionTimeMs;
            }
            
            std::cout << "  Summary: " << categoryPassed << "/" << categoryTotal 
                      << " passed (" << std::fixed << std::setprecision(1) 
                      << (100.0 * categoryPassed / categoryTotal) << "%) "
                      << "Total time: " << totalTime << "ms" << std::endl;
        }
        
        std::cout << "\nOVERALL RESULTS:" << std::endl;
        std::cout << "Total Tests: " << totalTests << std::endl;
        std::cout << "Passed: " << passedTests << " (" 
                  << std::fixed << std::setprecision(1) 
                  << (100.0 * passedTests / totalTests) << "%)" << std::endl;
        std::cout << "Failed: " << failedTests << " (" 
                  << std::fixed << std::setprecision(1) 
                  << (100.0 * failedTests / totalTests) << "%)" << std::endl;
        
        if (failedTests == 0) {
            std::cout << "\n🎉 ALL TESTS PASSED! 🎉" << std::endl;
        } else {
            std::cout << "\n❌ " << failedTests << " TESTS FAILED" << std::endl;
        }
        
        std::cout << std::string(60, '=') << std::endl;
    }
    
    bool allTestsPassed() const {
        return failedTests == 0;
    }
    
    void exportResults(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open " << filename << " for writing" << std::endl;
            return;
        }
        
        file << "Test Results Export\n";
        file << "Generated: " << getCurrentTimestamp() << "\n\n";
        
        for (const auto& result : results) {
            file << result.category << "::" << result.name << ","
                 << (result.passed ? "PASS" : "FAIL") << ","
                 << result.executionTimeMs << ","
                 << result.message << "\n";
        }
        
        file.close();
        std::cout << "Test results exported to " << filename << std::endl;
    }
    
private:
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};

// Test assertion macros
#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        std::cerr << "Assertion failed: " << #condition << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        return false; \
    }

#define ASSERT_FALSE(condition) \
    if (condition) { \
        std::cerr << "Assertion failed: !(" << #condition << ") at " << __FILE__ << ":" << __LINE__ << std::endl; \
        return false; \
    }

#define ASSERT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        std::cerr << "Assertion failed: " << #expected << " == " << #actual \
                  << " (expected: " << (expected) << ", actual: " << (actual) << ")" \
                  << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        return false; \
    }

#define ASSERT_NE(expected, actual) \
    if ((expected) == (actual)) { \
        std::cerr << "Assertion failed: " << #expected << " != " << #actual \
                  << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        return false; \
    }

#define ASSERT_GT(val1, val2) \
    if (!((val1) > (val2))) { \
        std::cerr << "Assertion failed: " << #val1 << " > " << #val2 \
                  << " (" << (val1) << " > " << (val2) << ")" \
                  << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        return false; \
    }

#define ASSERT_LT(val1, val2) \
    if (!((val1) < (val2))) { \
        std::cerr << "Assertion failed: " << #val1 << " < " << #val2 \
                  << " (" << (val1) << " < " << (val2) << ")" \
                  << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        return false; \
    }

#define ASSERT_CONTAINS(container, value) \
    if (std::find((container).begin(), (container).end(), (value)) == (container).end()) { \
        std::cerr << "Assertion failed: container contains " << #value \
                  << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        return false; \
    }

#define ASSERT_NOT_EMPTY(container) \
    if ((container).empty()) { \
        std::cerr << "Assertion failed: container is not empty" \
                  << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        return false; \
    }

// Performance testing utilities
class PerformanceBenchmark {
private:
    std::string name;
    std::chrono::high_resolution_clock::time_point startTime;
    std::vector<double> measurements;
    
public:
    PerformanceBenchmark(const std::string& benchmarkName) : name(benchmarkName) {}
    
    void start() {
        startTime = std::chrono::high_resolution_clock::now();
    }
    
    void stop() {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        measurements.push_back(duration.count() / 1000.0);  // Convert to milliseconds
    }
    
    void runBenchmark(std::function<void()> func, int iterations = 100) {
        measurements.clear();
        measurements.reserve(iterations);
        
        std::cout << "Running benchmark: " << name << " (" << iterations << " iterations)" << std::endl;
        
        for (int i = 0; i < iterations; ++i) {
            start();
            func();
            stop();
        }
        
        printResults();
    }
    
    void printResults() {
        if (measurements.empty()) {
            std::cout << "No measurements recorded for " << name << std::endl;
            return;
        }
        
        double sum = 0;
        double min = measurements[0];
        double max = measurements[0];
        
        for (double measurement : measurements) {
            sum += measurement;
            if (measurement < min) min = measurement;
            if (measurement > max) max = measurement;
        }
        
        double average = sum / measurements.size();
        
        // Calculate standard deviation
        double variance = 0;
        for (double measurement : measurements) {
            variance += (measurement - average) * (measurement - average);
        }
        variance /= measurements.size();
        double stddev = std::sqrt(variance);
        
        // Calculate percentiles
        std::vector<double> sorted = measurements;
        std::sort(sorted.begin(), sorted.end());
        double p50 = sorted[sorted.size() * 0.5];
        double p95 = sorted[sorted.size() * 0.95];
        double p99 = sorted[sorted.size() * 0.99];
        
        std::cout << "\nBenchmark Results: " << name << std::endl;
        std::cout << "  Iterations: " << measurements.size() << std::endl;
        std::cout << "  Average: " << std::fixed << std::setprecision(2) << average << "ms" << std::endl;
        std::cout << "  Min: " << min << "ms" << std::endl;
        std::cout << "  Max: " << max << "ms" << std::endl;
        std::cout << "  Std Dev: " << stddev << "ms" << std::endl;
        std::cout << "  P50: " << p50 << "ms" << std::endl;
        std::cout << "  P95: " << p95 << "ms" << std::endl;
        std::cout << "  P99: " << p99 << "ms" << std::endl;
        std::cout << std::endl;
    }
    
    double getAverageTime() const {
        if (measurements.empty()) return 0;
        double sum = 0;
        for (double measurement : measurements) {
            sum += measurement;
        }
        return sum / measurements.size();
    }
};

// Memory usage tracking
class MemoryTracker {
private:
    size_t initialMemory;
    size_t peakMemory;
    
public:
    MemoryTracker() {
        initialMemory = getCurrentMemoryUsage();
        peakMemory = initialMemory;
    }
    
    void checkpoint() {
        size_t current = getCurrentMemoryUsage();
        if (current > peakMemory) {
            peakMemory = current;
        }
    }
    
    void printReport() {
        size_t current = getCurrentMemoryUsage();
        checkpoint();
        
        std::cout << "Memory Usage Report:" << std::endl;
        std::cout << "  Initial: " << formatBytes(initialMemory) << std::endl;
        std::cout << "  Current: " << formatBytes(current) << std::endl;
        std::cout << "  Peak: " << formatBytes(peakMemory) << std::endl;
        std::cout << "  Increase: " << formatBytes(current - initialMemory) << std::endl;
    }
    
    bool isWithinLimit(size_t limitBytes) {
        return peakMemory <= limitBytes;
    }
    
private:
    size_t getCurrentMemoryUsage() {
        // Simple memory usage estimation (platform-specific implementation would be better)
        // This is a placeholder - in production, use platform-specific APIs
        return 0;  // Would implement actual memory tracking
    }
    
    std::string formatBytes(size_t bytes) {
        const char* units[] = {"B", "KB", "MB", "GB"};
        int unit = 0;
        double size = bytes;
        
        while (size >= 1024 && unit < 3) {
            size /= 1024;
            unit++;
        }
        
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << size << " " << units[unit];
        return oss.str();
    }
};

#endif // TEST_FRAMEWORK_H

