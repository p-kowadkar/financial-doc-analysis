#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <chrono>

// Simple TF-IDF implementation for testing
class TFIDFEmbedding {
public:
    std::vector<double> computeTFIDF(const std::string& document, const std::vector<std::string>& corpus) {
        std::vector<double> tfidf;
        
        if (document.empty() || corpus.empty()) {
            return tfidf;
        }
        
        // Simple word counting and TF-IDF calculation
        auto words = tokenize(document);
        std::map<std::string, double> tf;
        
        // Calculate term frequency
        for (const auto& word : words) {
            tf[word]++;
        }
        
        for (auto& pair : tf) {
            pair.second /= words.size();
        }
        
        // Simple TF-IDF score (simplified for testing)
        for (const auto& pair : tf) {
            double idf = log(corpus.size() / (1.0 + countDocumentsWithTerm(pair.first, corpus)));
            tfidf.push_back(pair.second * idf);
        }
        
        return tfidf;
    }
    
    double cosineSimilarity(const std::vector<double>& vec1, const std::vector<double>& vec2) {
        if (vec1.size() != vec2.size() || vec1.empty()) {
            return 0.0;
        }
        
        double dot = 0.0, norm1 = 0.0, norm2 = 0.0;
        
        for (size_t i = 0; i < vec1.size(); ++i) {
            dot += vec1[i] * vec2[i];
            norm1 += vec1[i] * vec1[i];
            norm2 += vec2[i] * vec2[i];
        }
        
        if (norm1 == 0.0 || norm2 == 0.0) {
            return 0.0;
        }
        
        return dot / (sqrt(norm1) * sqrt(norm2));
    }
    
private:
    std::vector<std::string> tokenize(const std::string& text) {
        std::vector<std::string> tokens;
        std::string word;
        
        for (char c : text) {
            if (isalnum(c)) {
                word += tolower(c);
            } else if (!word.empty()) {
                tokens.push_back(word);
                word.clear();
            }
        }
        
        if (!word.empty()) {
            tokens.push_back(word);
        }
        
        return tokens;
    }
    
    int countDocumentsWithTerm(const std::string& term, const std::vector<std::string>& corpus) {
        int count = 0;
        for (const auto& doc : corpus) {
            if (doc.find(term) != std::string::npos) {
                count++;
            }
        }
        return count;
    }
};

// Test functions
bool testTFIDFBasicFunctionality() {
    TFIDFEmbedding tfidf;
    
    std::vector<std::string> corpus = {
        "apple revenue growth financial performance",
        "microsoft cloud services revenue",
        "technology company financial results"
    };
    
    std::string document = "apple financial performance";
    auto embedding = tfidf.computeTFIDF(document, corpus);
    
    if (embedding.empty()) {
        std::cout << "FAIL: Empty TF-IDF embedding" << std::endl;
        return false;
    }
    
    std::cout << "PASS: Basic TF-IDF functionality test" << std::endl;
    return true;
}

bool testCosineSimilarity() {
    TFIDFEmbedding tfidf;
    
    std::vector<double> vec1 = {1.0, 2.0, 3.0};
    std::vector<double> vec2 = {1.0, 2.0, 3.0};
    std::vector<double> vec3 = {0.0, 0.0, 0.0};
    
    double similarity1 = tfidf.cosineSimilarity(vec1, vec2);
    double similarity2 = tfidf.cosineSimilarity(vec1, vec3);
    
    if (abs(similarity1 - 1.0) > 0.001) {
        std::cout << "FAIL: Identical vectors should have similarity 1.0" << std::endl;
        return false;
    }
    
    if (similarity2 != 0.0) {
        std::cout << "FAIL: Zero vector should have similarity 0.0" << std::endl;
        return false;
    }
    
    std::cout << "PASS: Cosine similarity test" << std::endl;
    return true;
}

bool testTFIDFErrorHandling() {
    TFIDFEmbedding tfidf;
    
    // Test empty document
    std::vector<std::string> corpus = {"test document"};
    auto embedding1 = tfidf.computeTFIDF("", corpus);
    
    if (!embedding1.empty()) {
        std::cout << "FAIL: Empty document should return empty embedding" << std::endl;
        return false;
    }
    
    // Test empty corpus
    std::vector<std::string> emptyCorpus;
    auto embedding2 = tfidf.computeTFIDF("test", emptyCorpus);
    
    if (!embedding2.empty()) {
        std::cout << "FAIL: Empty corpus should return empty embedding" << std::endl;
        return false;
    }
    
    std::cout << "PASS: Error handling test" << std::endl;
    return true;
}

bool testTFIDFPerformance() {
    TFIDFEmbedding tfidf;
    
    // Create larger corpus for performance testing
    std::vector<std::string> corpus;
    for (int i = 0; i < 100; ++i) {
        corpus.push_back("document " + std::to_string(i) + " with various financial terms revenue profit");
    }
    
    std::string document = "financial revenue analysis performance metrics";
    
    auto start = std::chrono::high_resolution_clock::now();
    auto embedding = tfidf.computeTFIDF(document, corpus);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    if (duration.count() > 1000) {  // Should complete within 1 second
        std::cout << "FAIL: TF-IDF computation too slow: " << duration.count() << "ms" << std::endl;
        return false;
    }
    
    if (embedding.empty()) {
        std::cout << "FAIL: Performance test returned empty embedding" << std::endl;
        return false;
    }
    
    std::cout << "PASS: Performance test (" << duration.count() << "ms)" << std::endl;
    return true;
}

// Main test runner
int main() {
    std::cout << "Running TF-IDF Embedding Unit Tests" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    int passed = 0;
    int total = 0;
    
    total++; if (testTFIDFBasicFunctionality()) passed++;
    total++; if (testCosineSimilarity()) passed++;
    total++; if (testTFIDFErrorHandling()) passed++;
    total++; if (testTFIDFPerformance()) passed++;
    
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
