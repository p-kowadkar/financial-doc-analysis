#ifndef NEURAL_EMBEDDINGS_H
#define NEURAL_EMBEDDINGS_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>

// Forward declarations for ONNX Runtime (to avoid header dependency issues)
namespace Ort {
    class Env;
    class Session;
    class SessionOptions;
    class Value;
    class MemoryInfo;
}

struct EmbeddingConfig {
    std::string modelPath;
    std::string modelType;  // "local_onnx", "openai", "huggingface", "deepseek"
    std::string apiKey;
    std::string apiEndpoint;
    int maxSequenceLength;
    int embeddingDimension;
    int batchSize;
    bool useGPU;
    bool quantized;
    std::string cacheDirectory;
    bool enableCaching;
};

struct TextEmbedding {
    std::vector<float> values;
    int dimension;
    std::string text;
    std::string modelUsed;
    double processingTime;
    bool fromCache;
};

struct EmbeddingBatch {
    std::vector<TextEmbedding> embeddings;
    std::vector<std::string> texts;
    double totalProcessingTime;
    int batchSize;
    std::string modelUsed;
};

class NeuralEmbeddingEngine {
private:
    EmbeddingConfig config;
    std::unique_ptr<Ort::Env> ortEnv;
    std::unique_ptr<Ort::Session> ortSession;
    std::unique_ptr<Ort::SessionOptions> sessionOptions;
    std::unique_ptr<Ort::MemoryInfo> memoryInfo;
    
    // Model metadata
    std::vector<std::string> inputNames;
    std::vector<std::string> outputNames;
    std::vector<std::vector<int64_t>> inputShapes;
    std::vector<std::vector<int64_t>> outputShapes;
    
    // Tokenization and preprocessing
    std::map<std::string, int> vocabulary;
    std::vector<std::string> reverseVocabulary;
    int padTokenId;
    int clsTokenId;
    int sepTokenId;
    int unkTokenId;
    
    // Caching system
    std::map<std::string, TextEmbedding> embeddingCache;
    std::string cacheFilePath;
    
    // Performance tracking
    mutable std::map<std::string, double> performanceMetrics;
    
    // Local ONNX inference methods
    bool initializeONNXRuntime();
    bool loadONNXModel(const std::string& modelPath);
    std::vector<int> tokenizeText(const std::string& text);
    std::vector<float> runONNXInference(const std::vector<int>& tokenIds);
    
    // Cloud API methods
    std::vector<float> callOpenAIEmbedding(const std::string& text);
    std::vector<float> callHuggingFaceEmbedding(const std::string& text);
    std::vector<float> callDeepSeekEmbedding(const std::string& text);
    std::string makeHttpRequest(const std::string& url, const std::string& payload, 
                               const std::map<std::string, std::string>& headers);
    
    // Utility methods
    std::string preprocessText(const std::string& text);
    std::vector<float> normalizeEmbedding(const std::vector<float>& embedding);
    std::string computeTextHash(const std::string& text);
    bool loadVocabulary(const std::string& vocabPath);
    
    // Caching methods
    bool loadEmbeddingCache();
    bool saveEmbeddingCache();
    bool isCached(const std::string& text);
    TextEmbedding getCachedEmbedding(const std::string& text);
    void cacheEmbedding(const std::string& text, const TextEmbedding& embedding);

public:
    explicit NeuralEmbeddingEngine(const EmbeddingConfig& config);
    ~NeuralEmbeddingEngine();
    
    // Initialization and configuration
    bool initialize();
    bool isInitialized() const;
    bool validateConfiguration() const;
    void updateConfig(const EmbeddingConfig& newConfig);
    
    // Single text embedding
    TextEmbedding embedText(const std::string& text);
    std::vector<float> embedTextSimple(const std::string& text);
    
    // Batch processing
    EmbeddingBatch embedTexts(const std::vector<std::string>& texts);
    std::vector<std::vector<float>> embedTextsSimple(const std::vector<std::string>& texts);
    
    // Similarity and search
    double cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b);
    double euclideanDistance(const std::vector<float>& a, const std::vector<float>& b);
    std::vector<std::pair<int, double>> findSimilar(const std::vector<float>& queryEmbedding,
                                                   const std::vector<std::vector<float>>& embeddings,
                                                   int topK = 10);
    
    // Model management
    std::vector<std::string> getAvailableModels() const;
    bool switchModel(const std::string& modelType, const std::string& modelPath = "");
    std::string getCurrentModel() const;
    
    // Performance and diagnostics
    std::map<std::string, double> getPerformanceMetrics() const;
    void resetPerformanceMetrics();
    std::string getSystemInfo() const;
    bool runDiagnostics();
    
    // Cache management
    void clearCache();
    int getCacheSize() const;
    bool exportCache(const std::string& filePath);
    bool importCache(const std::string& filePath);
    
    // Utility functions
    int getEmbeddingDimension() const;
    int getMaxSequenceLength() const;
    std::string getLastError() const;
    
    // Static utility methods
    static EmbeddingConfig createDefaultConfig();
    static std::vector<std::string> getSupportedModelTypes();
    static bool downloadModel(const std::string& modelName, const std::string& outputPath);
    static bool validateONNXModel(const std::string& modelPath);

private:
    mutable std::string lastError;
    bool initialized;
};

// Convenience functions for common use cases
namespace EmbeddingUtils {
    // Quick embedding with default configuration
    std::vector<float> quickEmbed(const std::string& text, 
                                 const std::string& modelType = "local_onnx");
    
    // Batch embedding with automatic batching
    std::vector<std::vector<float>> batchEmbed(const std::vector<std::string>& texts,
                                              const std::string& modelType = "local_onnx",
                                              int batchSize = 32);
    
    // Document similarity search
    std::vector<std::pair<int, double>> searchSimilarDocuments(
        const std::string& query,
        const std::vector<std::string>& documents,
        int topK = 5,
        const std::string& modelType = "local_onnx");
    
    // Embedding dimension reduction (for memory efficiency)
    std::vector<float> reduceEmbeddingDimension(const std::vector<float>& embedding,
                                               int targetDimension);
    
    // Embedding serialization
    std::string serializeEmbedding(const std::vector<float>& embedding);
    std::vector<float> deserializeEmbedding(const std::string& serialized);
}

#endif // NEURAL_EMBEDDINGS_H

