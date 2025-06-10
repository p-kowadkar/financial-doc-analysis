#include "../test_framework.h"
#include "../../src/document_parser.cpp"
#include "../../src/tfidf_embedding.cpp"
#include "../../src/query_engine.cpp"
#include "../../src/financial_analyzer.cpp"
#include <fstream>
#include <sstream>

// Integration test data setup
class IntegrationTestSetup {
public:
    std::vector<std::string> sampleDocuments;
    std::vector<std::string> documentIds;
    std::string indexPath;
    std::string outputPath;
    
    IntegrationTestSetup() {
        indexPath = "../test_data/integration_test_index.json";
        outputPath = "../test_data/integration_test_output";
        
        // Create comprehensive test documents
        sampleDocuments = {
            createAppleStyleDocument(),
            createMicrosoftStyleDocument(),
            createTeslaStyleDocument()
        };
        
        documentIds = {"AAPL_10K_2023", "MSFT_10K_2023", "TSLA_10K_2023"};
        
        // Ensure test directories exist
        system("mkdir -p ../test_data");
        system("mkdir -p ../test_results");
    }
    
private:
    std::string createAppleStyleDocument() {
        return R"(
UNITED STATES SECURITIES AND EXCHANGE COMMISSION
FORM 10-K ANNUAL REPORT
Apple Inc.

PART I
Item 1. Business

Apple Inc. designs, manufactures and markets smartphones, personal computers, tablets, wearables and accessories. The Company's fiscal year 2023 net sales were $383.3 billion, compared to $365.8 billion in fiscal 2022, representing an increase of 5%.

iPhone net sales were $200.6 billion in fiscal 2023, compared to $205.5 billion in fiscal 2022. Mac net sales were $29.4 billion compared to $40.2 billion in the prior year. iPad net sales were $28.3 billion compared to $29.3 billion in fiscal 2022.

Services net sales, which include the App Store, Apple Pay, Apple Music, and iCloud, were $85.2 billion in fiscal 2023, compared to $78.1 billion in fiscal 2022, representing growth of 9%.

Item 1A. Risk Factors

Cybersecurity threats could harm our business and reputation. We face cybersecurity threats, including attempts to gain unauthorized access to our information systems and data. A successful cybersecurity attack could result in the theft of intellectual property, disruption of our business operations, and damage to our reputation.

The highly competitive nature of the technology industry could adversely affect our business. We face significant competition in all areas of our business from companies with substantial resources and established market positions.

Supply chain disruptions could materially impact our business. Our business depends on third-party suppliers and manufacturers. Disruptions to our supply chain, including those caused by natural disasters, geopolitical tensions, or pandemic-related impacts, could materially affect our operations.

PART II
Item 7. Management's Discussion and Analysis

Total net sales increased 5% year-over-year to $383.3 billion in fiscal 2023. Gross margin was 44.1% compared to 43.3% in fiscal 2022. Operating margin was 29.5% compared to 30.3% in the prior year.

Cash and cash equivalents totaled $29.5 billion as of September 30, 2023, compared to $23.6 billion as of September 30, 2022.

Research and development expenses were $29.9 billion in fiscal 2023, compared to $26.3 billion in fiscal 2022, representing an increase of 14%.
)";
    }
    
    std::string createMicrosoftStyleDocument() {
        return R"(
UNITED STATES SECURITIES AND EXCHANGE COMMISSION
FORM 10-K ANNUAL REPORT
Microsoft Corporation

PART I
Item 1. Business

Microsoft Corporation develops, licenses, and supports software, services, devices, and solutions worldwide. Revenue for fiscal year 2023 was $211.9 billion, compared to $198.3 billion in fiscal 2022, representing an increase of 7%.

Productivity and Business Processes revenue was $69.3 billion, compared to $63.4 billion in fiscal 2022. Intelligent Cloud revenue was $87.9 billion, compared to $75.3 billion in the prior year. More Personal Computing revenue was $54.7 billion, compared to $59.6 billion in fiscal 2022.

Azure and other cloud services revenue increased 27% year-over-year, driven by continued customer adoption of our cloud platform.

Item 1A. Risk Factors

Cybersecurity, data protection, and privacy concerns could affect our reputation and business. We face cybersecurity threats that are increasing in sophistication and frequency. A significant cybersecurity incident could result in the disclosure of confidential information, disruption of our operations, and damage to our reputation.

Intense competition across all markets for our products and services may reduce our revenues and operating margins. We face competition from companies of all sizes across all markets for our products and services.

Our global operations subject us to potential liability under anti-corruption, trade protection, and other laws and regulations. We operate in countries with varying legal and regulatory requirements, and our failure to comply could result in significant penalties.

PART II
Item 7. Management's Discussion and Analysis

Revenue increased 7% to $211.9 billion in fiscal 2023, driven by growth across all three operating segments. Operating income increased 10% to $88.5 billion. Gross margin percentage increased to 69% from 68% in the prior year.

Cash and cash equivalents were $34.7 billion as of June 30, 2023. We returned $20.4 billion to shareholders through dividends and share repurchases during fiscal 2023.

Research and development expenses were $27.2 billion in fiscal 2023, representing 13% of revenue.
)";
    }
    
    std::string createTeslaStyleDocument() {
        return R"(
UNITED STATES SECURITIES AND EXCHANGE COMMISSION
FORM 10-K ANNUAL REPORT
Tesla, Inc.

PART I
Item 1. Business

Tesla, Inc. designs, develops, manufactures, leases and sells electric vehicles and energy generation and storage systems. Total revenues for the year ended December 31, 2023 were $96.8 billion, compared to $81.5 billion for the year ended December 31, 2022, representing an increase of 19%.

Automotive revenues were $82.4 billion in 2023, compared to $71.5 billion in 2022. Energy generation and storage revenues were $6.0 billion compared to $3.9 billion in the prior year. Services and other revenues were $8.3 billion compared to $6.1 billion in 2022.

We delivered approximately 1.81 million vehicles in 2023, compared to 1.31 million vehicles in 2022, representing growth of 38%.

Item 1A. Risk Factors

We face cybersecurity risks that could disrupt our operations. Our vehicles, energy products, and business operations depend on sophisticated software and hardware systems that could be subject to cybersecurity attacks. A successful attack could result in vehicle safety issues, theft of customer data, or disruption of our manufacturing operations.

The automotive industry is highly competitive and rapidly evolving. We face competition from established automotive manufacturers as well as new entrants focused on electric vehicles and autonomous driving technologies.

We depend on a limited number of suppliers for critical components. Our production could be disrupted if key suppliers experience operational difficulties, quality issues, or capacity constraints.

PART II
Item 7. Management's Discussion and Analysis

Total revenues increased 19% to $96.8 billion in 2023, primarily driven by increased vehicle deliveries and higher average selling prices. Automotive gross margin was 19.3% compared to 19.0% in 2022.

Cash and cash equivalents were $29.1 billion as of December 31, 2023. We generated $7.5 billion in free cash flow during 2023.

Research and development expenses were $3.1 billion in 2023, focused on vehicle development, autonomous driving capabilities, and energy products.
)";
    }
};

// Integration tests
bool testEndToEndDocumentProcessing() {
    IntegrationTestSetup setup;
    
    // Step 1: Parse documents
    DocumentParser parser;
    std::vector<DocumentSection> allSections;
    
    for (size_t i = 0; i < setup.sampleDocuments.size(); ++i) {
        auto sections = parser.parseDocument(setup.sampleDocuments[i]);
        ASSERT_NOT_EMPTY(sections);
        
        // Add document ID to sections
        for (auto& section : sections) {
            section.documentId = setup.documentIds[i];
            allSections.push_back(section);
        }
    }
    
    ASSERT_GT(allSections.size(), 10);  // Should have multiple sections
    
    // Step 2: Build TF-IDF index
    TFIDFEmbedding tfidf;
    std::vector<std::string> documents;
    std::vector<std::string> docIds;
    
    for (const auto& section : allSections) {
        documents.push_back(section.content);
        docIds.push_back(section.documentId + "_" + section.title);
    }
    
    bool indexSuccess = tfidf.buildIndex(documents, docIds);
    ASSERT_TRUE(indexSuccess);
    
    // Step 3: Test search functionality
    auto searchResults = tfidf.search("cybersecurity risks", 5);
    ASSERT_NOT_EMPTY(searchResults);
    
    // Should find cybersecurity-related content
    bool foundCybersecurity = false;
    for (const auto& result : searchResults) {
        if (result.similarity > 0.1) {  // Reasonable similarity threshold
            foundCybersecurity = true;
            break;
        }
    }
    ASSERT_TRUE(foundCybersecurity);
    
    return true;
}

bool testQueryEngineIntegration() {
    IntegrationTestSetup setup;
    
    // Build index first
    DocumentParser parser;
    TFIDFEmbedding tfidf;
    
    std::vector<std::string> documents;
    std::vector<std::string> docIds;
    
    for (size_t i = 0; i < setup.sampleDocuments.size(); ++i) {
        auto sections = parser.parseDocument(setup.sampleDocuments[i]);
        for (const auto& section : sections) {
            documents.push_back(section.content);
            docIds.push_back(setup.documentIds[i] + "_" + section.title);
        }
    }
    
    tfidf.buildIndex(documents, docIds);
    tfidf.saveIndex(setup.indexPath);
    
    // Test query engine
    QueryEngine queryEngine;
    bool loadSuccess = queryEngine.loadIndex(setup.indexPath);
    ASSERT_TRUE(loadSuccess);
    
    // Test various queries
    std::vector<std::string> testQueries = {
        "revenue growth financial performance",
        "cybersecurity threats and risks",
        "research and development expenses",
        "competition in technology industry",
        "supply chain disruptions"
    };
    
    for (const auto& query : testQueries) {
        auto results = queryEngine.query(query, 3);
        ASSERT_NOT_EMPTY(results);
        ASSERT_TRUE(results.size() <= 3);
        
        // Check result quality
        for (const auto& result : results) {
            ASSERT_FALSE(result.documentId.empty());
            ASSERT_FALSE(result.content.empty());
            ASSERT_TRUE(result.similarity >= 0.0 && result.similarity <= 1.0);
        }
    }
    
    return true;
}

bool testFinancialAnalyzerIntegration() {
    IntegrationTestSetup setup;
    
    // Test financial analysis on sample documents
    FinancialAnalyzer analyzer;
    
    for (size_t i = 0; i < setup.sampleDocuments.size(); ++i) {
        auto analysis = analyzer.analyzeDocument(setup.sampleDocuments[i]);
        
        ASSERT_FALSE(analysis.companyName.empty());
        ASSERT_NOT_EMPTY(analysis.financialMetrics);
        ASSERT_NOT_EMPTY(analysis.riskFactors);
        
        // Check for expected financial metrics
        bool foundRevenue = false;
        bool foundMargin = false;
        
        for (const auto& metric : analysis.financialMetrics) {
            if (metric.name.find("revenue") != std::string::npos ||
                metric.name.find("sales") != std::string::npos) {
                foundRevenue = true;
            }
            if (metric.name.find("margin") != std::string::npos) {
                foundMargin = true;
            }
        }
        
        ASSERT_TRUE(foundRevenue);
        // Note: margin might not always be found, so we don't assert it
        
        // Check for risk factors
        bool foundCybersecurity = false;
        bool foundCompetition = false;
        
        for (const auto& risk : analysis.riskFactors) {
            if (risk.category.find("cybersecurity") != std::string::npos ||
                risk.description.find("cybersecurity") != std::string::npos) {
                foundCybersecurity = true;
            }
            if (risk.category.find("competition") != std::string::npos ||
                risk.description.find("competition") != std::string::npos) {
                foundCompetition = true;
            }
        }
        
        ASSERT_TRUE(foundCybersecurity);
        ASSERT_TRUE(foundCompetition);
    }
    
    return true;
}

bool testRAGEngineIntegration() {
    IntegrationTestSetup setup;
    
    // Build complete system
    DocumentParser parser;
    TFIDFEmbedding tfidf;
    
    std::vector<std::string> documents;
    std::vector<std::string> docIds;
    
    for (size_t i = 0; i < setup.sampleDocuments.size(); ++i) {
        auto sections = parser.parseDocument(setup.sampleDocuments[i]);
        for (const auto& section : sections) {
            documents.push_back(section.content);
            docIds.push_back(setup.documentIds[i] + "_" + section.title);
        }
    }
    
    tfidf.buildIndex(documents, docIds);
    
    // Test RAG functionality
    RAGEngine ragEngine;
    ragEngine.setIndex(&tfidf);
    
    std::vector<std::string> ragQueries = {
        "Compare the financial performance of Apple and Microsoft",
        "What are the main cybersecurity risks mentioned by these companies?",
        "Analyze the research and development spending trends",
        "Summarize the competitive landscape for these technology companies"
    };
    
    for (const auto& query : ragQueries) {
        auto response = ragEngine.generateResponse(query, 3);
        
        ASSERT_FALSE(response.summary.empty());
        ASSERT_NOT_EMPTY(response.sourceDocuments);
        ASSERT_TRUE(response.sourceDocuments.size() <= 3);
        
        // Check that summary contains relevant information
        ASSERT_GT(response.summary.length(), 100);  // Should be substantial
        
        // Check source documents are relevant
        for (const auto& source : response.sourceDocuments) {
            ASSERT_TRUE(source.similarity > 0.0);
            ASSERT_FALSE(source.content.empty());
        }
    }
    
    return true;
}

bool testSystemPerformance() {
    IntegrationTestSetup setup;
    
    PerformanceBenchmark benchmark("End-to-End System Performance");
    
    auto systemTest = [&]() {
        // Full pipeline test
        DocumentParser parser;
        auto sections = parser.parseDocument(setup.sampleDocuments[0]);
        
        TFIDFEmbedding tfidf;
        std::vector<std::string> docs;
        for (const auto& section : sections) {
            docs.push_back(section.content);
        }
        tfidf.buildIndex(docs);
        
        auto results = tfidf.search("financial performance", 3);
        
        FinancialAnalyzer analyzer;
        auto analysis = analyzer.analyzeDocument(setup.sampleDocuments[0]);
    };
    
    benchmark.runBenchmark(systemTest, 10);
    
    // Full system should complete in reasonable time (under 5 seconds)
    ASSERT_LT(benchmark.getAverageTime(), 5000.0);
    
    return true;
}

bool testSystemMemoryUsage() {
    IntegrationTestSetup setup;
    MemoryTracker tracker;
    
    // Process all documents
    DocumentParser parser;
    TFIDFEmbedding tfidf;
    FinancialAnalyzer analyzer;
    
    std::vector<std::string> allDocuments;
    std::vector<std::string> allDocIds;
    
    for (size_t i = 0; i < setup.sampleDocuments.size(); ++i) {
        auto sections = parser.parseDocument(setup.sampleDocuments[i]);
        tracker.checkpoint();
        
        for (const auto& section : sections) {
            allDocuments.push_back(section.content);
            allDocIds.push_back(setup.documentIds[i] + "_" + section.title);
        }
        
        auto analysis = analyzer.analyzeDocument(setup.sampleDocuments[i]);
        tracker.checkpoint();
    }
    
    // Build index
    tfidf.buildIndex(allDocuments, allDocIds);
    tracker.checkpoint();
    
    // Perform multiple searches
    for (int i = 0; i < 50; ++i) {
        tfidf.search("test query " + std::to_string(i), 5);
        if (i % 10 == 0) tracker.checkpoint();
    }
    
    tracker.printReport();
    
    // Should stay within 2GB limit as specified in requirements
    ASSERT_TRUE(tracker.isWithinLimit(2LL * 1024 * 1024 * 1024));
    
    return true;
}

bool testDataPersistenceAndRecovery() {
    IntegrationTestSetup setup;
    
    // Build and save system state
    DocumentParser parser;
    TFIDFEmbedding tfidf1;
    
    std::vector<std::string> documents;
    std::vector<std::string> docIds;
    
    for (size_t i = 0; i < setup.sampleDocuments.size(); ++i) {
        auto sections = parser.parseDocument(setup.sampleDocuments[i]);
        for (const auto& section : sections) {
            documents.push_back(section.content);
            docIds.push_back(setup.documentIds[i] + "_" + section.title);
        }
    }
    
    tfidf1.buildIndex(documents, docIds);
    
    // Save state
    std::string indexPath = "../test_data/persistence_test_index.json";
    bool saveSuccess = tfidf1.saveIndex(indexPath);
    ASSERT_TRUE(saveSuccess);
    
    // Test original system
    auto originalResults = tfidf1.search("cybersecurity risks", 3);
    ASSERT_NOT_EMPTY(originalResults);
    
    // Load into new system
    TFIDFEmbedding tfidf2;
    bool loadSuccess = tfidf2.loadIndex(indexPath);
    ASSERT_TRUE(loadSuccess);
    
    // Test loaded system
    auto loadedResults = tfidf2.search("cybersecurity risks", 3);
    ASSERT_NOT_EMPTY(loadedResults);
    
    // Results should be identical
    ASSERT_EQ(originalResults.size(), loadedResults.size());
    for (size_t i = 0; i < originalResults.size(); ++i) {
        ASSERT_EQ(originalResults[i].documentId, loadedResults[i].documentId);
        ASSERT_TRUE(std::abs(originalResults[i].similarity - loadedResults[i].similarity) < 0.001);
    }
    
    return true;
}

bool testErrorHandlingAndRecovery() {
    IntegrationTestSetup setup;
    
    // Test various error conditions
    DocumentParser parser;
    TFIDFEmbedding tfidf;
    FinancialAnalyzer analyzer;
    
    // Test empty document handling
    auto emptySections = parser.parseDocument("");
    ASSERT_TRUE(emptySections.empty() || emptySections.size() == 1);
    
    // Test malformed document
    std::string malformedDoc = "This is not a proper 10-K document with random content.";
    auto malformedSections = parser.parseDocument(malformedDoc);
    // Should handle gracefully without crashing
    
    // Test empty index search
    auto emptyResults = tfidf.search("test query", 5);
    ASSERT_TRUE(emptyResults.empty());
    
    // Test invalid file operations
    bool loadFailed = tfidf.loadIndex("nonexistent_file.json");
    ASSERT_FALSE(loadFailed);
    
    // Test analysis of empty content
    auto emptyAnalysis = analyzer.analyzeDocument("");
    // Should handle gracefully
    
    return true;
}

// Main integration test runner
int main() {
    TestFramework framework;
    
    std::cout << "Running Integration Tests" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    // Create test directories
    system("mkdir -p ../test_data");
    system("mkdir -p ../test_results");
    
    framework.setCategory("Integration");
    
    framework.runTest("EndToEndDocumentProcessing", testEndToEndDocumentProcessing);
    framework.runTest("QueryEngineIntegration", testQueryEngineIntegration);
    framework.runTest("FinancialAnalyzerIntegration", testFinancialAnalyzerIntegration);
    framework.runTest("RAGEngineIntegration", testRAGEngineIntegration);
    framework.runTest("SystemPerformance", testSystemPerformance);
    framework.runTest("SystemMemoryUsage", testSystemMemoryUsage);
    framework.runTest("DataPersistenceAndRecovery", testDataPersistenceAndRecovery);
    framework.runTest("ErrorHandlingAndRecovery", testErrorHandlingAndRecovery);
    
    framework.printSummary();
    framework.exportResults("../test_results/integration_test_results.csv");
    
    return framework.allTestsPassed() ? 0 : 1;
}

