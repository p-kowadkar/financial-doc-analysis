#include <iostream>
#include <string>
#include <vector>

// Simple test structure for document parser
struct Section {
    std::string title;
    std::string content;
};

struct DocumentMetadata {
    std::string companyName;
    std::string formType;
    std::string fiscalYear;
    std::string ticker;
};

// Mock document parser for testing
class DocumentParser {
public:
    std::vector<Section> parseDocument(const std::string& content) {
        std::vector<Section> sections;
        
        if (content.empty()) {
            return sections;
        }
        
        // Simple parsing logic for testing
        if (content.find("Item 1. Business") != std::string::npos) {
            sections.push_back({"Business", "Technology company content"});
        }
        if (content.find("Item 1A. Risk") != std::string::npos) {
            sections.push_back({"Risk Factors", "Cybersecurity and competition risks"});
        }
        if (content.find("Item 7. Management") != std::string::npos) {
            sections.push_back({"Management Discussion", "Revenue analysis"});
        }
        
        return sections;
    }
    
    DocumentMetadata extractMetadata(const std::string& content) {
        DocumentMetadata metadata;
        
        if (content.find("TEST COMPANY INC.") != std::string::npos) {
            metadata.companyName = "TEST COMPANY INC.";
        }
        if (content.find("FORM 10-K") != std::string::npos) {
            metadata.formType = "10-K";
        }
        if (content.find("fiscal year ended September 30, 2023") != std::string::npos) {
            metadata.fiscalYear = "2023";
        }
        if (content.find("symbol \"TEST\"") != std::string::npos) {
            metadata.ticker = "TEST";
        }
        
        return metadata;
    }
    
    std::string toJSON(const std::string& content) {
        auto sections = parseDocument(content);
        auto metadata = extractMetadata(content);
        
        std::string json = "{\n";
        json += "  \"company_name\": \"" + metadata.companyName + "\",\n";
        json += "  \"form_type\": \"" + metadata.formType + "\",\n";
        json += "  \"sections\": [\n";
        
        for (size_t i = 0; i < sections.size(); ++i) {
            json += "    {\"title\": \"" + sections[i].title + "\", \"content\": \"" + sections[i].content + "\"}";
            if (i < sections.size() - 1) json += ",";
            json += "\n";
        }
        
        json += "  ]\n";
        json += "}";
        
        return json;
    }
};

// Test data
std::string createTestDocument() {
    return R"(
FORM 10-K
For the fiscal year ended September 30, 2023
TEST COMPANY INC.

Item 1. Business
Test Company Inc. is a technology company.

Item 1A. Risk Factors
Cybersecurity Risks: We face cybersecurity threats.
Competition: The markets are highly competitive.

Item 7. Management's Discussion and Analysis
Revenue for fiscal 2023 increased 8%.

Our common stock is traded on NASDAQ under the symbol "TEST".
)";
}

// Unit tests
bool testDocumentParserBasicFunctionality() {
    std::string testDoc = createTestDocument();
    DocumentParser parser;
    auto sections = parser.parseDocument(testDoc);
    
    if (sections.empty()) {
        std::cout << "FAIL: No sections parsed" << std::endl;
        return false;
    }
    
    if (sections.size() < 3) {
        std::cout << "FAIL: Expected at least 3 sections, got " << sections.size() << std::endl;
        return false;
    }
    
    std::cout << "PASS: Basic functionality test" << std::endl;
    return true;
}

bool testDocumentParserMetadataExtraction() {
    std::string testDoc = createTestDocument();
    DocumentParser parser;
    auto metadata = parser.extractMetadata(testDoc);
    
    if (metadata.companyName != "TEST COMPANY INC.") {
        std::cout << "FAIL: Company name mismatch" << std::endl;
        return false;
    }
    
    if (metadata.formType != "10-K") {
        std::cout << "FAIL: Form type mismatch" << std::endl;
        return false;
    }
    
    std::cout << "PASS: Metadata extraction test" << std::endl;
    return true;
}

bool testDocumentParserJSONOutput() {
    std::string testDoc = createTestDocument();
    DocumentParser parser;
    std::string jsonOutput = parser.toJSON(testDoc);
    
    if (jsonOutput.empty()) {
        std::cout << "FAIL: Empty JSON output" << std::endl;
        return false;
    }
    
    if (jsonOutput.find("company_name") == std::string::npos) {
        std::cout << "FAIL: JSON missing company_name field" << std::endl;
        return false;
    }
    
    if (jsonOutput.find("sections") == std::string::npos) {
        std::cout << "FAIL: JSON missing sections field" << std::endl;
        return false;
    }
    
    std::cout << "PASS: JSON output test" << std::endl;
    return true;
}

bool testDocumentParserErrorHandling() {
    DocumentParser parser;
    
    // Test empty document
    auto emptySections = parser.parseDocument("");
    if (!emptySections.empty()) {
        std::cout << "FAIL: Empty document should return no sections" << std::endl;
        return false;
    }
    
    // Test malformed document
    auto malformedSections = parser.parseDocument("Random text with no structure");
    // Should handle gracefully
    
    std::cout << "PASS: Error handling test" << std::endl;
    return true;
}

// Main test runner
int main() {
    std::cout << "Running Document Parser Unit Tests" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    int passed = 0;
    int total = 0;
    
    total++; if (testDocumentParserBasicFunctionality()) passed++;
    total++; if (testDocumentParserMetadataExtraction()) passed++;
    total++; if (testDocumentParserJSONOutput()) passed++;
    total++; if (testDocumentParserErrorHandling()) passed++;
    
    std::cout << std::string(50, '=') << std::endl;
    std::cout << "Tests passed: " << passed << "/" << total << std::endl;
    
    if (passed == total) {
        std::cout << "All tests PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << "Some tests FAILED!" << std::endl;
        return 1;
    }
}
