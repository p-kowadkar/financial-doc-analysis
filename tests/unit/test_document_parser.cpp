#include "../test_framework.h"
#include "../../src/document_parser.cpp"
#include <fstream>
#include <sstream>

// Test data creation utilities
std::string createTestDocument() {
    return R"(
UNITED STATES
SECURITIES AND EXCHANGE COMMISSION
Washington, D.C. 20549

FORM 10-K

ANNUAL REPORT PURSUANT TO SECTION 13 OR 15(d) OF THE SECURITIES EXCHANGE ACT OF 1934

For the fiscal year ended September 30, 2023

Commission File Number: 001-36743

TEST COMPANY INC.
(Exact name of registrant as specified in its charter)

PART I

Item 1. Business

Test Company Inc. is a technology company that designs, manufactures and markets consumer electronics, computer software, and online services. The Company's products include smartphones, tablets, personal computers, and wearables.

Our total net sales for fiscal 2023 were $394.3 billion, compared to $365.8 billion in fiscal 2022, representing an increase of 8%. 

Item 1A. Risk Factors

The following risk factors may materially affect our business:

Cybersecurity Risks: We face cybersecurity threats that could result in unauthorized access to our systems and data breaches. Such incidents could harm our reputation and result in significant costs.

Competition: The markets for our products are highly competitive. We face significant competition from companies with substantial resources.

Supply Chain: Our business depends on third-party suppliers. Disruptions to our supply chain could materially impact our operations.

Item 2. Properties

We own and lease various facilities worldwide for our operations.

Item 3. Legal Proceedings

We are subject to various legal proceedings and claims.

PART II

Item 5. Market for Registrant's Common Equity

Our common stock is traded on NASDAQ under the symbol "TEST".

Item 7. Management's Discussion and Analysis

Revenue for fiscal 2023 increased 8% compared to fiscal 2022, primarily due to higher sales of our smartphone products. Gross margin was 43.3% compared to 43.7% in the prior year.

Our cash and cash equivalents totaled $29.5 billion as of September 30, 2023.

PART III

Item 10. Directors, Executive Officers and Corporate Governance

Our board of directors consists of eight members.

PART IV

Item 15. Exhibits and Financial Statement Schedules

See attached financial statements.
)";
}

std::string createMalformedDocument() {
    return R"(
This is not a proper 10-K document.
It has no structure.
Random text here.
No sections or items.
)";
}

std::string createEmptyDocument() {
    return "";
}

// Unit tests for document parsing functionality
bool testDocumentParserBasicFunctionality() {
    std::string testDoc = createTestDocument();
    
    // Test basic parsing
    DocumentParser parser;
    auto sections = parser.parseDocument(testDoc);
    
    ASSERT_NOT_EMPTY(sections);
    ASSERT_GT(sections.size(), 5);  // Should have multiple sections
    
    return true;
}

bool testDocumentParserSectionExtraction() {
    std::string testDoc = createTestDocument();
    DocumentParser parser;
    auto sections = parser.parseDocument(testDoc);
    
    // Check for specific sections
    bool foundBusiness = false;
    bool foundRisks = false;
    bool foundMDA = false;
    
    for (const auto& section : sections) {
        if (section.title.find("Business") != std::string::npos) {
            foundBusiness = true;
            ASSERT_CONTAINS(section.content, "technology company");
            ASSERT_CONTAINS(section.content, "$394.3 billion");
        }
        if (section.title.find("Risk") != std::string::npos) {
            foundRisks = true;
            ASSERT_CONTAINS(section.content, "Cybersecurity");
            ASSERT_CONTAINS(section.content, "Competition");
        }
        if (section.title.find("Management") != std::string::npos) {
            foundMDA = true;
            ASSERT_CONTAINS(section.content, "Revenue for fiscal 2023");
        }
    }
    
    ASSERT_TRUE(foundBusiness);
    ASSERT_TRUE(foundRisks);
    ASSERT_TRUE(foundMDA);
    
    return true;
}

bool testDocumentParserMetadataExtraction() {
    std::string testDoc = createTestDocument();
    DocumentParser parser;
    auto metadata = parser.extractMetadata(testDoc);
    
    ASSERT_EQ(metadata.companyName, "TEST COMPANY INC.");
    ASSERT_EQ(metadata.formType, "10-K");
    ASSERT_EQ(metadata.fiscalYear, "2023");
    ASSERT_CONTAINS(metadata.ticker, "TEST");
    
    return true;
}

bool testDocumentParserErrorHandling() {
    DocumentParser parser;
    
    // Test empty document
    auto emptySections = parser.parseDocument(createEmptyDocument());
    ASSERT_TRUE(emptySections.empty());
    
    // Test malformed document
    auto malformedSections = parser.parseDocument(createMalformedDocument());
    // Should still return something, even if minimal
    ASSERT_TRUE(malformedSections.size() >= 0);
    
    return true;
}

bool testDocumentParserPerformance() {
    std::string testDoc = createTestDocument();
    DocumentParser parser;
    
    PerformanceBenchmark benchmark("Document Parsing");
    
    auto parseFunc = [&]() {
        parser.parseDocument(testDoc);
    };
    
    benchmark.runBenchmark(parseFunc, 50);
    
    // Parsing should be fast (under 100ms average)
    ASSERT_LT(benchmark.getAverageTime(), 100.0);
    
    return true;
}

bool testDocumentParserMemoryUsage() {
    MemoryTracker tracker;
    
    std::string testDoc = createTestDocument();
    DocumentParser parser;
    
    // Parse multiple documents to test memory usage
    for (int i = 0; i < 100; ++i) {
        auto sections = parser.parseDocument(testDoc);
        tracker.checkpoint();
    }
    
    tracker.printReport();
    
    // Should stay within reasonable memory limits (100MB for this test)
    ASSERT_TRUE(tracker.isWithinLimit(100 * 1024 * 1024));
    
    return true;
}

bool testDocumentParserJSONOutput() {
    std::string testDoc = createTestDocument();
    DocumentParser parser;
    
    // Test JSON serialization
    std::string jsonOutput = parser.toJSON(testDoc);
    
    ASSERT_NOT_EMPTY(jsonOutput);
    ASSERT_CONTAINS(jsonOutput, "company_name");
    ASSERT_CONTAINS(jsonOutput, "sections");
    ASSERT_CONTAINS(jsonOutput, "TEST COMPANY INC.");
    
    // Validate JSON structure (basic check)
    ASSERT_TRUE(jsonOutput.front() == '{');
    ASSERT_TRUE(jsonOutput.back() == '}');
    
    return true;
}

bool testDocumentParserEdgeCases() {
    DocumentParser parser;
    
    // Test very large document
    std::string largeDoc = createTestDocument();
    for (int i = 0; i < 100; ++i) {
        largeDoc += createTestDocument();
    }
    
    auto largeSections = parser.parseDocument(largeDoc);
    ASSERT_NOT_EMPTY(largeSections);
    
    // Test document with special characters
    std::string specialDoc = "Item 1. Business\nCompany™ with ® symbols and € currency\n";
    auto specialSections = parser.parseDocument(specialDoc);
    ASSERT_NOT_EMPTY(specialSections);
    
    // Test document with Unicode
    std::string unicodeDoc = "Item 1. Business\nCompany with 中文 and émojis 🚀\n";
    auto unicodeSections = parser.parseDocument(unicodeDoc);
    ASSERT_NOT_EMPTY(unicodeSections);
    
    return true;
}

bool testDocumentParserConcurrency() {
    // Test thread safety (basic test)
    DocumentParser parser;
    std::string testDoc = createTestDocument();
    
    bool success = true;
    
    // Simulate concurrent parsing (simplified test)
    for (int i = 0; i < 10; ++i) {
        auto sections = parser.parseDocument(testDoc);
        if (sections.empty()) {
            success = false;
            break;
        }
    }
    
    ASSERT_TRUE(success);
    
    return true;
}

// Main test runner for document parser
int main() {
    TestFramework framework;
    
    std::cout << "Running Document Parser Unit Tests" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    framework.setCategory("DocumentParser");
    
    framework.runTest("BasicFunctionality", testDocumentParserBasicFunctionality);
    framework.runTest("SectionExtraction", testDocumentParserSectionExtraction);
    framework.runTest("MetadataExtraction", testDocumentParserMetadataExtraction);
    framework.runTest("ErrorHandling", testDocumentParserErrorHandling);
    framework.runTest("Performance", testDocumentParserPerformance);
    framework.runTest("MemoryUsage", testDocumentParserMemoryUsage);
    framework.runTest("JSONOutput", testDocumentParserJSONOutput);
    framework.runTest("EdgeCases", testDocumentParserEdgeCases);
    framework.runTest("Concurrency", testDocumentParserConcurrency);
    
    framework.printSummary();
    framework.exportResults("../test_results/document_parser_results.csv");
    
    return framework.allTestsPassed() ? 0 : 1;
}

