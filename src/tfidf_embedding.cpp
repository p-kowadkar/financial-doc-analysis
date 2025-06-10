#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <regex>
#include <iomanip>

class SimpleJSONParser {
public:
    static std::string extractStringValue(const std::string& json, const std::string& key) {
        std::string pattern = "\"" + key + "\"\\s*:\\s*\"([^\"]+)\"";
        std::regex regex(pattern);
        std::smatch match;
        
        if (std::regex_search(json, match, regex)) {
            return match[1].str();
        }
        return "";
    }
    
    static std::vector<std::pair<std::string, std::string>> extractSections(const std::string& json) {
        std::vector<std::pair<std::string, std::string>> sections;
        
        // Find sections array
        size_t sectionsStart = json.find("\"sections\"");
        if (sectionsStart == std::string::npos) return sections;
        
        size_t arrayStart = json.find("[", sectionsStart);
        if (arrayStart == std::string::npos) return sections;
        
        size_t arrayEnd = json.find("]", arrayStart);
        if (arrayEnd == std::string::npos) return sections;
        
        std::string sectionsArray = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
        
        // Simple section extraction using brace matching
        int braceCount = 0;
        size_t start = 0;
        
        for (size_t i = 0; i < sectionsArray.length(); ++i) {
            if (sectionsArray[i] == '{') {
                if (braceCount == 0) start = i;
                braceCount++;
            } else if (sectionsArray[i] == '}') {
                braceCount--;
                if (braceCount == 0) {
                    std::string sectionJson = sectionsArray.substr(start, i - start + 1);
                    std::string title = extractStringValue(sectionJson, "title");
                    std::string content = extractStringValue(sectionJson, "content");
                    
                    if (!title.empty() && !content.empty()) {
                        sections.push_back({title, content});
                    }
                }
            }
        }
        
        return sections;
    }
};

class TFIDFEmbedding {
private:
    std::unordered_map<std::string, std::unordered_map<std::string, double>> documentTermFreq;
    std::unordered_map<std::string, double> inverseDocumentFreq;
    std::unordered_map<std::string, std::string> documentContent;
    std::vector<std::string> vocabulary;
    int totalDocuments;

    std::vector<std::string> tokenize(const std::string& text) {
        std::vector<std::string> tokens;
        std::string word;
        
        for (char c : text) {
            if (std::isalnum(c)) {
                word += std::tolower(c);
            } else if (!word.empty()) {
                if (word.length() > 2) {  // Filter short words
                    tokens.push_back(word);
                }
                word.clear();
            }
        }
        
        if (!word.empty() && word.length() > 2) {
            tokens.push_back(word);
        }
        
        return tokens;
    }

    void calculateTF(const std::string& docId, const std::vector<std::string>& tokens) {
        std::unordered_map<std::string, int> termCount;
        
        for (const std::string& token : tokens) {
            termCount[token]++;
        }
        
        int totalTerms = tokens.size();
        if (totalTerms > 0) {
            for (const auto& pair : termCount) {
                documentTermFreq[docId][pair.first] = static_cast<double>(pair.second) / totalTerms;
            }
        }
    }

    void calculateIDF() {
        std::unordered_map<std::string, int> documentFreq;
        
        for (const auto& doc : documentTermFreq) {
            for (const auto& term : doc.second) {
                documentFreq[term.first]++;
            }
        }
        
        for (const auto& pair : documentFreq) {
            inverseDocumentFreq[pair.first] = std::log(static_cast<double>(totalDocuments) / pair.second);
        }
        
        vocabulary.clear();
        for (const auto& pair : documentFreq) {
            vocabulary.push_back(pair.first);
        }
        std::sort(vocabulary.begin(), vocabulary.end());
    }

    double cosineSimilarity(const std::unordered_map<std::string, double>& vec1,
                           const std::unordered_map<std::string, double>& vec2) {
        double dotProduct = 0.0;
        double norm1 = 0.0;
        double norm2 = 0.0;
        
        for (const auto& pair : vec1) {
            norm1 += pair.second * pair.second;
            auto it = vec2.find(pair.first);
            if (it != vec2.end()) {
                dotProduct += pair.second * it->second;
            }
        }
        
        for (const auto& pair : vec2) {
            norm2 += pair.second * pair.second;
        }
        
        if (norm1 == 0.0 || norm2 == 0.0) return 0.0;
        
        return dotProduct / (std::sqrt(norm1) * std::sqrt(norm2));
    }

public:
    TFIDFEmbedding() : totalDocuments(0) {}

    bool addDocument(const std::string& docId, const std::string& content) {
        std::vector<std::string> tokens = tokenize(content);
        if (tokens.empty()) return false;
        
        documentContent[docId] = content;
        calculateTF(docId, tokens);
        totalDocuments++;
        
        return true;
    }

    void buildIndex() {
        calculateIDF();
        std::cout << "Index built successfully!" << std::endl;
        std::cout << "Total documents: " << totalDocuments << std::endl;
        std::cout << "Vocabulary size: " << vocabulary.size() << std::endl;
    }

    std::vector<std::pair<std::string, double>> searchSimilar(const std::string& query, int topK = 5) {
        std::vector<std::string> queryTokens = tokenize(query);
        std::unordered_map<std::string, double> queryTFIDF;
        
        // Calculate query TF
        std::unordered_map<std::string, int> queryTermCount;
        for (const std::string& token : queryTokens) {
            queryTermCount[token]++;
        }
        
        int totalQueryTerms = queryTokens.size();
        if (totalQueryTerms > 0) {
            for (const auto& pair : queryTermCount) {
                double tf = static_cast<double>(pair.second) / totalQueryTerms;
                auto idfIt = inverseDocumentFreq.find(pair.first);
                if (idfIt != inverseDocumentFreq.end()) {
                    queryTFIDF[pair.first] = tf * idfIt->second;
                }
            }
        }
        
        // Calculate similarities
        std::vector<std::pair<std::string, double>> similarities;
        
        for (const auto& doc : documentTermFreq) {
            std::unordered_map<std::string, double> docTFIDF;
            
            for (const auto& term : doc.second) {
                auto idfIt = inverseDocumentFreq.find(term.first);
                if (idfIt != inverseDocumentFreq.end()) {
                    docTFIDF[term.first] = term.second * idfIt->second;
                }
            }
            
            double similarity = cosineSimilarity(queryTFIDF, docTFIDF);
            similarities.push_back({doc.first, similarity});
        }
        
        // Sort by similarity
        std::sort(similarities.begin(), similarities.end(),
                 [](const auto& a, const auto& b) { return a.second > b.second; });
        
        // Return top K
        if (topK < static_cast<int>(similarities.size())) {
            similarities.resize(topK);
        }
        
        return similarities;
    }

    bool saveIndex(const std::string& filePath) {
        std::ofstream file(filePath);
        if (!file.is_open()) return false;
        
        file << "{\n";
        file << "  \"total_documents\": " << totalDocuments << ",\n";
        file << "  \"vocabulary_size\": " << vocabulary.size() << ",\n";
        file << "  \"documents\": {\n";
        
        bool first = true;
        for (const auto& doc : documentContent) {
            if (!first) file << ",\n";
            std::string preview = doc.second.substr(0, 100);
            // Escape quotes in preview
            size_t pos = 0;
            while ((pos = preview.find("\"", pos)) != std::string::npos) {
                preview.replace(pos, 1, "\\\"");
                pos += 2;
            }
            file << "    \"" << doc.first << "\": \"" << preview << "...\"";
            first = false;
        }
        
        file << "\n  }\n";
        file << "}";
        file.close();
        
        return true;
    }

    bool loadFromDirectory(const std::string& dirPath) {
        // Look for JSON files in the directory
        std::vector<std::string> jsonFiles = {
            "apple_10k_2023_structured.json", 
            "microsoft_10k_2024_structured.json"
        };
        
        for (const std::string& filename : jsonFiles) {
            std::string fullPath = dirPath + "/" + filename;
            std::ifstream file(fullPath);
            
            if (file.is_open()) {
                std::string content((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
                file.close();
                
                std::string companyName = SimpleJSONParser::extractStringValue(content, "company_name");
                auto sections = SimpleJSONParser::extractSections(content);
                
                for (const auto& section : sections) {
                    std::string docId = companyName + "_" + section.first;
                    addDocument(docId, section.second);
                }
                
                std::cout << "Loaded " << sections.size() << " sections from " << filename << std::endl;
            }
        }
        
        if (totalDocuments > 0) {
            buildIndex();
            return true;
        }
        
        return false;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "TF-IDF Embedding System - Windows Compatible" << std::endl;
        std::cout << "Usage:" << std::endl;
        std::cout << "  Build index: " << argv[0] << " build <input_dir> <output_file>" << std::endl;
        std::cout << "  Search: " << argv[0] << " search <index_file> <query> [top_k]" << std::endl;
        return 1;
    }

    std::string command = argv[1];
    TFIDFEmbedding embedding;

    if (command == "build") {
        if (argc != 4) {
            std::cout << "Usage: " << argv[0] << " build <input_dir> <output_file>" << std::endl;
            return 1;
        }

        std::string inputDir = argv[2];
        std::string outputFile = argv[3];

        std::cout << "Loading documents from: " << inputDir << std::endl;
        
        if (embedding.loadFromDirectory(inputDir)) {
            if (embedding.saveIndex(outputFile)) {
                std::cout << "Index saved to: " << outputFile << std::endl;
            } else {
                std::cout << "Failed to save index" << std::endl;
                return 1;
            }
        } else {
            std::cout << "Failed to load documents" << std::endl;
            return 1;
        }

    } else if (command == "search") {
        if (argc < 4) {
            std::cout << "Usage: " << argv[0] << " search <index_file> <query> [top_k]" << std::endl;
            return 1;
        }

        std::string indexFile = argv[2];
        std::string query = argv[3];
        int topK = (argc > 4) ? std::stoi(argv[4]) : 5;

        // For this simplified version, we'll rebuild the index
        std::cout << "Rebuilding index for search..." << std::endl;
        std::string inputDir = "../output";  // Default directory
        
        if (embedding.loadFromDirectory(inputDir)) {
            std::cout << "Searching for: \"" << query << "\"" << std::endl;
            
            auto results = embedding.searchSimilar(query, topK);
            
            std::cout << "\nTop " << results.size() << " results:" << std::endl;
            for (size_t i = 0; i < results.size(); ++i) {
                std::cout << (i + 1) << ". " << results[i].first 
                         << " (similarity: " << std::fixed << std::setprecision(4) 
                         << results[i].second << ")" << std::endl;
            }
        } else {
            std::cout << "Failed to load index" << std::endl;
            return 1;
        }

    } else {
        std::cout << "Unknown command: " << command << std::endl;
        return 1;
    }

    return 0;
}

