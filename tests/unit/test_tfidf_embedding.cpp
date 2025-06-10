#include "../test_framework.h"
#include "../../src/tfidf_embedding.cpp"
#include <fstream>
#include <cmath>

// Test data creation for TF-IDF testing
std::vector<std::string> createTestDocuments() {
    return {
        "The company reported strong financial performance with revenue growth of 15% year over year.",
        "Cybersecurity risks remain a significant concern for technology companies in the current environment.",
        "The quarterly earnings showed improved profit margins and increased market share in key segments.",
        "Supply chain disruptions continue to impact manufacturing operations and delivery schedules.",
        "Investment in research and development increased by 20% to support innovation initiatives."
    };
}

std::vector<std::string> createFinancialQueries() {
    return {
        "revenue growth financial performance",
        "cybersecurity risks technology",
        "profit margins earnings",
        "supply chain manufacturing",
        "research development innovation"
    };
}

// Unit tests for TF-IDF embedding functionality
bool testTFIDFBasicFunctionality() {
    TFIDFEmbedding tfidf;
    std::vector<std::string> docs = createTestDocuments();
    
    // Build index
    bool success = tfidf.buildIndex(docs);
    ASSERT_TRUE(success);
    
    // Check vocabulary size
    auto vocab = tfidf.getVocabulary();
    ASSERT_GT(vocab.size(), 10);  // Should have reasonable vocabulary
    
    return true;
}

bool testTFIDFDocumentAddition() {
    TFIDFEmbedding tfidf;
    std::vector<std::string> docs = createTestDocuments();
    
    // Add documents one by one
    for (size_t i = 0; i < docs.size(); ++i) {
        std::string docId = "doc_" + std::to_string(i);
        bool success = tfidf.addDocument(docId, docs[i]);
        ASSERT_TRUE(success);
    }
    
    // Check document count
    ASSERT_EQ(tfidf.getDocumentCount(), docs.size());
    
    return true;
}

bool testTFIDFVectorGeneration() {
    TFIDFEmbedding tfidf;
    std::vector<std::string> docs = createTestDocuments();
    tfidf.buildIndex(docs);
    
    // Generate vector for a document
    auto vector = tfidf.getDocumentVector("doc_0");
    ASSERT_NOT_EMPTY(vector);
    
    // Check vector properties
    double magnitude = 0;
    for (double val : vector) {
        magnitude += val * val;
    }
    magnitude = std::sqrt(magnitude);
    
    ASSERT_GT(magnitude, 0);  // Vector should have non-zero magnitude
    
    return true;
}

bool testTFIDFSimilaritySearch() {
    TFIDFEmbedding tfidf;
    std::vector<std::string> docs = createTestDocuments();
    tfidf.buildIndex(docs);
    
    std::vector<std::string> queries = createFinancialQueries();
    
    for (const auto& query : queries) {
        auto results = tfidf.search(query, 3);
        
        ASSERT_NOT_EMPTY(results);
        ASSERT_TRUE(results.size() <= 3);
        
        // Check that results are sorted by similarity (descending)
        for (size_t i = 1; i < results.size(); ++i) {
            ASSERT_TRUE(results[i-1].similarity >= results[i].similarity);
        }
        
        // Check similarity scores are in valid range [0, 1]
        for (const auto& result : results) {
            ASSERT_TRUE(result.similarity >= 0.0 && result.similarity <= 1.0);
        }
    }
    
    return true;
}

bool testTFIDFTokenization() {
    TFIDFEmbedding tfidf;
    
    // Test various tokenization scenarios
    std::string text1 = "Hello, world! This is a test.";
    auto tokens1 = tfidf.tokenize(text1);
    ASSERT_CONTAINS(tokens1, "hello");
    ASSERT_CONTAINS(tokens1, "world");
    ASSERT_CONTAINS(tokens1, "test");
    
    // Test financial terms
    std::string text2 = "Revenue increased by $1.5 billion (15%) year-over-year.";
    auto tokens2 = tfidf.tokenize(text2);
    ASSERT_CONTAINS(tokens2, "revenue");
    ASSERT_CONTAINS(tokens2, "increased");
    ASSERT_CONTAINS(tokens2, "billion");
    
    // Test special characters and numbers
    std::string text3 = "Q3 2023 earnings: $2.5B revenue, 12.3% growth";
    auto tokens3 = tfidf.tokenize(text3);
    ASSERT_NOT_EMPTY(tokens3);
    
    return true;
}

bool testTFIDFStopWordRemoval() {
    TFIDFEmbedding tfidf;
    
    std::string text = "The company is a leading provider of the best solutions in the market.";
    auto tokens = tfidf.tokenize(text);
    
    // Common stop words should be removed
    ASSERT_FALSE(std::find(tokens.begin(), tokens.end(), "the") != tokens.end());
    ASSERT_FALSE(std::find(tokens.begin(), tokens.end(), "is") != tokens.end());
    ASSERT_FALSE(std::find(tokens.begin(), tokens.end(), "a") != tokens.end());
    ASSERT_FALSE(std::find(tokens.begin(), tokens.end(), "in") != tokens.end());
    
    // Important words should remain
    ASSERT_CONTAINS(tokens, "company");
    ASSERT_CONTAINS(tokens, "leading");
    ASSERT_CONTAINS(tokens, "provider");
    ASSERT_CONTAINS(tokens, "solutions");
    
    return true;
}

bool testTFIDFPerformance() {
    TFIDFEmbedding tfidf;
    
    // Create larger dataset for performance testing
    std::vector<std::string> largeDocs;
    std::vector<std::string> baseDocs = createTestDocuments();
    
    for (int i = 0; i < 100; ++i) {
        for (const auto& doc : baseDocs) {
            largeDocs.push_back(doc + " Document " + std::to_string(i));
        }
    }
    
    PerformanceBenchmark indexBenchmark("TF-IDF Index Building");
    indexBenchmark.runBenchmark([&]() {
        tfidf.buildIndex(largeDocs);
    }, 5);
    
    // Index building should be reasonable (under 1 second for 500 docs)
    ASSERT_LT(indexBenchmark.getAverageTime(), 1000.0);
    
    // Test search performance
    PerformanceBenchmark searchBenchmark("TF-IDF Search");
    searchBenchmark.runBenchmark([&]() {
        tfidf.search("financial performance revenue", 10);
    }, 100);
    
    // Search should be fast (under 50ms)
    ASSERT_LT(searchBenchmark.getAverageTime(), 50.0);
    
    return true;
}

bool testTFIDFMemoryUsage() {
    MemoryTracker tracker;
    
    TFIDFEmbedding tfidf;
    std::vector<std::string> docs = createTestDocuments();
    
    // Build index and perform operations
    tfidf.buildIndex(docs);
    tracker.checkpoint();
    
    // Perform multiple searches
    for (int i = 0; i < 100; ++i) {
        tfidf.search("test query " + std::to_string(i), 5);
        if (i % 10 == 0) tracker.checkpoint();
    }
    
    tracker.printReport();
    
    // Should stay within reasonable memory limits (50MB for this test)
    ASSERT_TRUE(tracker.isWithinLimit(50 * 1024 * 1024));
    
    return true;
}

bool testTFIDFSerialization() {
    TFIDFEmbedding tfidf;
    std::vector<std::string> docs = createTestDocuments();
    tfidf.buildIndex(docs);
    
    // Save index
    std::string indexPath = "../test_data/test_tfidf_index.json";
    bool saveSuccess = tfidf.saveIndex(indexPath);
    ASSERT_TRUE(saveSuccess);
    
    // Load index into new instance
    TFIDFEmbedding tfidf2;
    bool loadSuccess = tfidf2.loadIndex(indexPath);
    ASSERT_TRUE(loadSuccess);
    
    // Compare search results
    std::string query = "financial performance";
    auto results1 = tfidf.search(query, 3);
    auto results2 = tfidf2.search(query, 3);
    
    ASSERT_EQ(results1.size(), results2.size());
    
    // Results should be identical
    for (size_t i = 0; i < results1.size(); ++i) {
        ASSERT_EQ(results1[i].documentId, results2[i].documentId);
        ASSERT_TRUE(std::abs(results1[i].similarity - results2[i].similarity) < 0.001);
    }
    
    return true;
}

bool testTFIDFEdgeCases() {
    TFIDFEmbedding tfidf;
    
    // Test empty document
    std::vector<std::string> emptyDocs = {""};
    bool success = tfidf.buildIndex(emptyDocs);
    ASSERT_TRUE(success);  // Should handle gracefully
    
    // Test single word document
    std::vector<std::string> singleWordDocs = {"revenue"};
    success = tfidf.buildIndex(singleWordDocs);
    ASSERT_TRUE(success);
    
    // Test very long document
    std::string longDoc = "";
    for (int i = 0; i < 1000; ++i) {
        longDoc += "word" + std::to_string(i) + " ";
    }
    std::vector<std::string> longDocs = {longDoc};
    success = tfidf.buildIndex(longDocs);
    ASSERT_TRUE(success);
    
    // Test duplicate documents
    std::vector<std::string> duplicateDocs = {
        "same document content",
        "same document content",
        "different content"
    };
    success = tfidf.buildIndex(duplicateDocs);
    ASSERT_TRUE(success);
    
    return true;
}

bool testTFIDFQueryEnhancement() {
    TFIDFEmbedding tfidf;
    std::vector<std::string> docs = createTestDocuments();
    tfidf.buildIndex(docs);
    
    // Test query expansion/enhancement
    std::string shortQuery = "revenue";
    std::string expandedQuery = "revenue growth financial performance earnings";
    
    auto results1 = tfidf.search(shortQuery, 5);
    auto results2 = tfidf.search(expandedQuery, 5);
    
    ASSERT_NOT_EMPTY(results1);
    ASSERT_NOT_EMPTY(results2);
    
    // Expanded query should potentially return more relevant results
    // (This is a basic test - in practice, we'd need more sophisticated evaluation)
    
    return true;
}

// Main test runner for TF-IDF embedding
int main() {
    TestFramework framework;
    
    std::cout << "Running TF-IDF Embedding Unit Tests" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    // Create test data directory
    system("mkdir -p ../test_data");
    
    framework.setCategory("TFIDFEmbedding");
    
    framework.runTest("BasicFunctionality", testTFIDFBasicFunctionality);
    framework.runTest("DocumentAddition", testTFIDFDocumentAddition);
    framework.runTest("VectorGeneration", testTFIDFVectorGeneration);
    framework.runTest("SimilaritySearch", testTFIDFSimilaritySearch);
    framework.runTest("Tokenization", testTFIDFTokenization);
    framework.runTest("StopWordRemoval", testTFIDFStopWordRemoval);
    framework.runTest("Performance", testTFIDFPerformance);
    framework.runTest("MemoryUsage", testTFIDFMemoryUsage);
    framework.runTest("Serialization", testTFIDFSerialization);
    framework.runTest("EdgeCases", testTFIDFEdgeCases);
    framework.runTest("QueryEnhancement", testTFIDFQueryEnhancement);
    
    framework.printSummary();
    framework.exportResults("../test_results/tfidf_embedding_results.csv");
    
    return framework.allTestsPassed() ? 0 : 1;
}

