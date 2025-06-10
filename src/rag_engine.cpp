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
#include <chrono>

class Logger {
private:
    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

public:
    void log(const std::string& level, const std::string& message) {
        std::cout << "[" << getTimestamp() << "] [" << level << "] " << message << std::endl;
    }
};

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
        
        size_t sectionsStart = json.find("\"sections\"");
        if (sectionsStart == std::string::npos) return sections;
        
        size_t arrayStart = json.find("[", sectionsStart);
        if (arrayStart == std::string::npos) return sections;
        
        size_t arrayEnd = json.find("]", arrayStart);
        if (arrayEnd == std::string::npos) return sections;
        
        std::string sectionsArray = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
        
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
                if (word.length() > 2) {
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
    }

    std::vector<std::pair<std::string, double>> searchSimilar(const std::string& query, int topK = 5) {
        std::vector<std::string> queryTokens = tokenize(query);
        std::unordered_map<std::string, double> queryTFIDF;
        
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
        
        std::sort(similarities.begin(), similarities.end(),
                 [](const auto& a, const auto& b) { return a.second > b.second; });
        
        if (topK < static_cast<int>(similarities.size())) {
            similarities.resize(topK);
        }
        
        return similarities;
    }

    std::string getDocumentContent(const std::string& docId) {
        auto it = documentContent.find(docId);
        if (it != documentContent.end()) {
            return it->second;
        }
        return "";
    }

    bool loadFromDirectory(const std::string& dirPath) {
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
            }
        }
        
        if (totalDocuments > 0) {
            buildIndex();
            return true;
        }
        
        return false;
    }
};

class RAGEngine {
private:
    TFIDFEmbedding embedding;
    Logger logger;

    std::string extractiveSummarize(const std::vector<std::pair<std::string, double>>& results, const std::string& query) {
        std::stringstream summary;
        
        summary << "Based on the query \"" << query << "\", here are the key findings:\n\n";
        
        for (size_t i = 0; i < results.size(); ++i) {
            std::string content = embedding.getDocumentContent(results[i].first);
            
            // Extract key sentences related to the query
            std::vector<std::string> sentences = extractKeySentences(content, query, 2);
            
            summary << "From " << results[i].first << " (relevance: " 
                   << std::fixed << std::setprecision(2) << results[i].second << "):\n";
            
            for (const std::string& sentence : sentences) {
                summary << "- " << sentence << "\n";
            }
            summary << "\n";
        }
        
        return summary.str();
    }

    std::vector<std::string> extractKeySentences(const std::string& content, const std::string& query, int maxSentences) {
        std::vector<std::string> sentences;
        std::vector<std::string> queryWords = tokenize(query);
        
        // Split content into sentences (simple approach)
        std::regex sentenceRegex("[.!?]+\\s+");
        std::sregex_token_iterator iter(content.begin(), content.end(), sentenceRegex, -1);
        std::sregex_token_iterator end;
        
        std::vector<std::pair<std::string, int>> sentenceScores;
        
        for (; iter != end; ++iter) {
            std::string sentence = iter->str();
            if (sentence.length() > 50) {  // Filter short sentences
                int score = 0;
                std::string lowerSentence = sentence;
                std::transform(lowerSentence.begin(), lowerSentence.end(), lowerSentence.begin(), ::tolower);
                
                for (const std::string& word : queryWords) {
                    if (lowerSentence.find(word) != std::string::npos) {
                        score++;
                    }
                }
                
                if (score > 0) {
                    sentenceScores.push_back({sentence, score});
                }
            }
        }
        
        // Sort by score and take top sentences
        std::sort(sentenceScores.begin(), sentenceScores.end(),
                 [](const auto& a, const auto& b) { return a.second > b.second; });
        
        int count = std::min(maxSentences, static_cast<int>(sentenceScores.size()));
        for (int i = 0; i < count; ++i) {
            sentences.push_back(sentenceScores[i].first);
        }
        
        return sentences;
    }

    std::vector<std::string> tokenize(const std::string& text) {
        std::vector<std::string> tokens;
        std::string word;
        
        for (char c : text) {
            if (std::isalnum(c)) {
                word += std::tolower(c);
            } else if (!word.empty()) {
                if (word.length() > 2) {
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

public:
    bool loadIndex(const std::string& indexPath) {
        logger.log("INFO", "Initializing RAGEngine with index: " + indexPath);
        logger.log("INFO", "Using extractive summarization (no external dependencies)");
        
        std::string inputDir = "../output";
        if (embedding.loadFromDirectory(inputDir)) {
            logger.log("INFO", "Successfully loaded index");
            return true;
        } else {
            logger.log("ERROR", "Failed to load index");
            return false;
        }
    }

    void executeRAG(const std::string& query, const std::string& outputFile, int topK) {
        logger.log("INFO", "Executing query: \"" + query + "\"");
        
        auto start = std::chrono::high_resolution_clock::now();
        auto results = embedding.searchSimilar(query, topK);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        logger.log("INFO", "Retrieved " + std::to_string(results.size()) + " results for query");
        
        logger.log("INFO", "Generating extractive summary for " + std::to_string(results.size()) + " results");
        std::string summary = extractiveSummarize(results, query);
        
        std::ofstream outFile(outputFile);
        if (!outFile.is_open()) {
            logger.log("ERROR", "Could not create output file: " + outputFile);
            return;
        }

        outFile << "# RAG Analysis Results\n\n";
        outFile << "**Query:** " << query << "\n\n";
        outFile << "**Analysis Method:** Extractive Summarization\n\n";
        outFile << "**Number of Documents Analyzed:** " << results.size() << "\n\n";
        outFile << "## Summary\n\n";
        outFile << summary << "\n\n";
        
        outFile << "## Detailed Results\n\n";
        for (size_t i = 0; i < results.size(); ++i) {
            outFile << "### " << (i + 1) << ". " << results[i].first << "\n";
            outFile << "**Similarity Score:** " << std::fixed << std::setprecision(4) << results[i].second << "\n\n";
            
            std::string content = embedding.getDocumentContent(results[i].first);
            if (content.length() > 800) {
                content = content.substr(0, 800) + "...";
            }
            outFile << content << "\n\n";
            outFile << "---\n\n";
        }
        
        outFile.close();
        logger.log("INFO", "Query executed successfully in " + std::to_string(duration.count()) + "ms");
        logger.log("INFO", "Results saved to " + outputFile);
    }
};

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "RAG Engine - Windows Compatible (Extractive Summarization)" << std::endl;
        std::cout << "Usage: " << argv[0] << " <index_file> <query> <output_file> [top_k]" << std::endl;
        std::cout << "Example: " << argv[0] << " index.json \"financial risks\" results.md 3" << std::endl;
        return 1;
    }

    std::string indexFile = argv[1];
    std::string query = argv[2];
    std::string outputFile = argv[3];
    int topK = (argc > 4) ? std::stoi(argv[4]) : 3;

    RAGEngine engine;
    
    if (engine.loadIndex(indexFile)) {
        engine.executeRAG(query, outputFile, topK);
    } else {
        std::cerr << "Failed to load index" << std::endl;
        return 1;
    }

    return 0;
}

