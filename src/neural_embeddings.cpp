#include "neural_embeddings.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <regex>
#include <cmath>
#include <random>
#include <curl/curl.h>

// ONNX Runtime includes (with fallback for systems without ONNX)
#ifdef ONNX_RUNTIME_AVAILABLE
#include <onnxruntime_cxx_api.h>
#else
// Stub implementations when ONNX Runtime is not available
namespace Ort {
    class Env { public: Env(int, const char*) {} };
    class Session { public: Session(Env&, const char*, void*) {} };
    class SessionOptions { public: SessionOptions() {} };
    class Value { public: static Value CreateTensor(void*, void*, size_t, void*, size_t) { return Value(); } };
    class MemoryInfo { public: static MemoryInfo CreateCpu(int, int) { return MemoryInfo(); } };
}
#endif

// HTTP callback for libcurl
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

NeuralEmbeddingEngine::NeuralEmbeddingEngine(const EmbeddingConfig& config) 
    : config(config), initialized(false) {
    
    // Initialize performance metrics
    performanceMetrics["total_embeddings"] = 0;
    performanceMetrics["total_processing_time"] = 0;
    performanceMetrics["cache_hits"] = 0;
    performanceMetrics["cache_misses"] = 0;
    performanceMetrics["api_calls"] = 0;
    performanceMetrics["local_inferences"] = 0;
    
    // Set cache file path
    cacheFilePath = config.cacheDirectory + "/embedding_cache.json";
}

NeuralEmbeddingEngine::~NeuralEmbeddingEngine() {
    if (config.enableCaching) {
        saveEmbeddingCache();
    }
}

bool NeuralEmbeddingEngine::initialize() {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    try {
        // Load embedding cache if enabled
        if (config.enableCaching) {
            loadEmbeddingCache();
        }
        
        // Initialize based on model type
        if (config.modelType == "local_onnx") {
            #ifdef ONNX_RUNTIME_AVAILABLE
            if (!initializeONNXRuntime()) {
                lastError = "Failed to initialize ONNX Runtime";
                return false;
            }
            
            if (!loadONNXModel(config.modelPath)) {
                lastError = "Failed to load ONNX model: " + config.modelPath;
                return false;
            }
            #else
            lastError = "ONNX Runtime not available. Please install ONNX Runtime or use cloud API.";
            std::cerr << "WARNING: " << lastError << std::endl;
            std::cerr << "Falling back to cloud API mode..." << std::endl;
            
            // Auto-fallback to cloud API
            if (!config.apiKey.empty()) {
                config.modelType = "openai";  // Default fallback
            } else {
                return false;
            }
            #endif
        }
        
        // Validate cloud API configuration
        if (config.modelType != "local_onnx") {
            if (config.apiKey.empty()) {
                lastError = "API key required for cloud embedding service: " + config.modelType;
                return false;
            }
        }
        
        initialized = true;
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        performanceMetrics["initialization_time"] = duration.count();
        
        std::cout << "[INFO] Neural embedding engine initialized successfully" << std::endl;
        std::cout << "[INFO] Model type: " << config.modelType << std::endl;
        std::cout << "[INFO] Embedding dimension: " << config.embeddingDimension << std::endl;
        std::cout << "[INFO] Initialization time: " << duration.count() << "ms" << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        lastError = "Initialization failed: " + std::string(e.what());
        return false;
    }
}

#ifdef ONNX_RUNTIME_AVAILABLE
bool NeuralEmbeddingEngine::initializeONNXRuntime() {
    try {
        // Create ONNX Runtime environment
        ortEnv = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "FinancialAnalysis");
        
        // Create session options
        sessionOptions = std::make_unique<Ort::SessionOptions>();
        sessionOptions->SetIntraOpNumThreads(1);
        sessionOptions->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
        
        // Enable GPU if requested and available
        if (config.useGPU) {
            try {
                Ort::ThrowOnError(OrtSessionOptionsAppendExecutionProvider_CUDA(*sessionOptions, 0));
                std::cout << "[INFO] CUDA GPU acceleration enabled" << std::endl;
            } catch (const std::exception& e) {
                std::cout << "[WARNING] GPU not available, falling back to CPU: " << e.what() << std::endl;
                config.useGPU = false;
            }
        }
        
        // Create memory info
        memoryInfo = std::make_unique<Ort::MemoryInfo>(
            Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault));
        
        return true;
        
    } catch (const std::exception& e) {
        lastError = "ONNX Runtime initialization failed: " + std::string(e.what());
        return false;
    }
}

bool NeuralEmbeddingEngine::loadONNXModel(const std::string& modelPath) {
    try {
        // Load the ONNX model
        ortSession = std::make_unique<Ort::Session>(*ortEnv, modelPath.c_str(), *sessionOptions);
        
        // Get input/output information
        size_t numInputNodes = ortSession->GetInputCount();
        size_t numOutputNodes = ortSession->GetOutputCount();
        
        inputNames.clear();
        outputNames.clear();
        inputShapes.clear();
        outputShapes.clear();
        
        // Get input names and shapes
        for (size_t i = 0; i < numInputNodes; i++) {
            char* inputName = ortSession->GetInputName(i, Ort::AllocatorWithDefaultOptions());
            inputNames.push_back(std::string(inputName));
            
            Ort::TypeInfo inputTypeInfo = ortSession->GetInputTypeInfo(i);
            auto inputTensorInfo = inputTypeInfo.GetTensorTypeAndShapeInfo();
            auto inputShape = inputTensorInfo.GetShape();
            inputShapes.push_back(inputShape);
        }
        
        // Get output names and shapes
        for (size_t i = 0; i < numOutputNodes; i++) {
            char* outputName = ortSession->GetOutputName(i, Ort::AllocatorWithDefaultOptions());
            outputNames.push_back(std::string(outputName));
            
            Ort::TypeInfo outputTypeInfo = ortSession->GetOutputTypeInfo(i);
            auto outputTensorInfo = outputTypeInfo.GetTensorTypeAndShapeInfo();
            auto outputShape = outputTensorInfo.GetShape();
            outputShapes.push_back(outputShape);
        }
        
        std::cout << "[INFO] ONNX model loaded successfully" << std::endl;
        std::cout << "[INFO] Input nodes: " << numInputNodes << std::endl;
        std::cout << "[INFO] Output nodes: " << numOutputNodes << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        lastError = "Failed to load ONNX model: " + std::string(e.what());
        return false;
    }
}

std::vector<float> NeuralEmbeddingEngine::runONNXInference(const std::vector<int>& tokenIds) {
    try {
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Prepare input tensor
        std::vector<int64_t> inputShape = {1, static_cast<int64_t>(tokenIds.size())};
        std::vector<int64_t> inputData(tokenIds.begin(), tokenIds.end());
        
        Ort::Value inputTensor = Ort::Value::CreateTensor<int64_t>(
            *memoryInfo, inputData.data(), inputData.size(), inputShape.data(), inputShape.size());
        
        // Run inference
        const char* inputNamesPtr[] = {inputNames[0].c_str()};
        const char* outputNamesPtr[] = {outputNames[0].c_str()};
        
        auto outputTensors = ortSession->Run(Ort::RunOptions{nullptr}, 
                                           inputNamesPtr, &inputTensor, 1,
                                           outputNamesPtr, 1);
        
        // Extract output
        float* outputData = outputTensors[0].GetTensorMutableData<float>();
        auto outputShape = outputTensors[0].GetTensorTypeAndShapeInfo().GetShape();
        
        size_t outputSize = 1;
        for (auto dim : outputShape) {
            outputSize *= dim;
        }
        
        std::vector<float> embedding(outputData, outputData + outputSize);
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        performanceMetrics["local_inferences"]++;
        performanceMetrics["total_processing_time"] += duration.count();
        
        return normalizeEmbedding(embedding);
        
    } catch (const std::exception& e) {
        lastError = "ONNX inference failed: " + std::string(e.what());
        return {};
    }
}
#else
// Stub implementations when ONNX Runtime is not available
bool NeuralEmbeddingEngine::initializeONNXRuntime() {
    lastError = "ONNX Runtime not compiled in this build";
    return false;
}

bool NeuralEmbeddingEngine::loadONNXModel(const std::string& modelPath) {
    lastError = "ONNX Runtime not available";
    return false;
}

std::vector<float> NeuralEmbeddingEngine::runONNXInference(const std::vector<int>& tokenIds) {
    lastError = "ONNX Runtime not available";
    return {};
}
#endif

std::vector<int> NeuralEmbeddingEngine::tokenizeText(const std::string& text) {
    // Simple tokenization (for production, use proper tokenizer like SentencePiece)
    std::vector<int> tokens;
    
    // Add CLS token
    tokens.push_back(clsTokenId);
    
    // Simple word-level tokenization
    std::istringstream iss(preprocessText(text));
    std::string word;
    
    while (iss >> word && tokens.size() < config.maxSequenceLength - 1) {
        auto it = vocabulary.find(word);
        if (it != vocabulary.end()) {
            tokens.push_back(it->second);
        } else {
            tokens.push_back(unkTokenId);  // Unknown token
        }
    }
    
    // Add SEP token
    tokens.push_back(sepTokenId);
    
    // Pad to max length
    while (tokens.size() < config.maxSequenceLength) {
        tokens.push_back(padTokenId);
    }
    
    return tokens;
}

std::string NeuralEmbeddingEngine::preprocessText(const std::string& text) {
    std::string processed = text;
    
    // Convert to lowercase
    std::transform(processed.begin(), processed.end(), processed.begin(), ::tolower);
    
    // Remove extra whitespace
    std::regex multipleSpaces("\\s+");
    processed = std::regex_replace(processed, multipleSpaces, " ");
    
    // Trim
    processed.erase(0, processed.find_first_not_of(" \t\n\r"));
    processed.erase(processed.find_last_not_of(" \t\n\r") + 1);
    
    return processed;
}

std::vector<float> NeuralEmbeddingEngine::callOpenAIEmbedding(const std::string& text) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Prepare JSON payload
    std::string payload = R"({
        "input": ")" + text + R"(",
        "model": "text-embedding-ada-002"
    })";
    
    // Prepare headers
    std::map<std::string, std::string> headers;
    headers["Content-Type"] = "application/json";
    headers["Authorization"] = "Bearer " + config.apiKey;
    
    // Make API call
    std::string response = makeHttpRequest("https://api.openai.com/v1/embeddings", payload, headers);
    
    if (response.empty()) {
        lastError = "OpenAI API call failed";
        return {};
    }
    
    // Parse response (simplified JSON parsing)
    std::regex embeddingPattern(R"("embedding":\s*\[([\d\.,\s-]+)\])");
    std::smatch match;
    
    if (!std::regex_search(response, match, embeddingPattern)) {
        lastError = "Failed to parse OpenAI embedding response";
        return {};
    }
    
    std::string embeddingStr = match[1].str();
    std::vector<float> embedding;
    
    // Parse comma-separated float values
    std::istringstream iss(embeddingStr);
    std::string value;
    
    while (std::getline(iss, value, ',')) {
        try {
            embedding.push_back(std::stof(value));
        } catch (const std::exception& e) {
            // Skip invalid values
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    performanceMetrics["api_calls"]++;
    performanceMetrics["total_processing_time"] += duration.count();
    
    return embedding;
}

std::vector<float> NeuralEmbeddingEngine::callHuggingFaceEmbedding(const std::string& text) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Prepare JSON payload for HuggingFace Inference API
    std::string payload = R"({
        "inputs": ")" + text + R"("
    })";
    
    // Prepare headers
    std::map<std::string, std::string> headers;
    headers["Content-Type"] = "application/json";
    headers["Authorization"] = "Bearer " + config.apiKey;
    
    // Use sentence-transformers model endpoint
    std::string endpoint = config.apiEndpoint.empty() ? 
        "https://api-inference.huggingface.co/models/sentence-transformers/all-MiniLM-L6-v2" :
        config.apiEndpoint;
    
    std::string response = makeHttpRequest(endpoint, payload, headers);
    
    if (response.empty()) {
        lastError = "HuggingFace API call failed";
        return {};
    }
    
    // Parse response array
    std::vector<float> embedding;
    std::regex numberPattern(R"(-?\d+\.?\d*(?:[eE][+-]?\d+)?)");
    std::sregex_iterator iter(response.begin(), response.end(), numberPattern);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        try {
            embedding.push_back(std::stof(iter->str()));
        } catch (const std::exception& e) {
            // Skip invalid values
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    performanceMetrics["api_calls"]++;
    performanceMetrics["total_processing_time"] += duration.count();
    
    return embedding;
}

std::vector<float> NeuralEmbeddingEngine::callDeepSeekEmbedding(const std::string& text) {
    // DeepSeek API implementation (similar structure to OpenAI)
    auto startTime = std::chrono::high_resolution_clock::now();
    
    std::string payload = R"({
        "input": ")" + text + R"(",
        "model": "deepseek-embedding"
    })";
    
    std::map<std::string, std::string> headers;
    headers["Content-Type"] = "application/json";
    headers["Authorization"] = "Bearer " + config.apiKey;
    
    std::string endpoint = config.apiEndpoint.empty() ? 
        "https://api.deepseek.com/v1/embeddings" :
        config.apiEndpoint;
    
    std::string response = makeHttpRequest(endpoint, payload, headers);
    
    if (response.empty()) {
        lastError = "DeepSeek API call failed";
        return {};
    }
    
    // Parse similar to OpenAI format
    std::regex embeddingPattern(R"("embedding":\s*\[([\d\.,\s-]+)\])");
    std::smatch match;
    
    if (!std::regex_search(response, match, embeddingPattern)) {
        lastError = "Failed to parse DeepSeek embedding response";
        return {};
    }
    
    std::string embeddingStr = match[1].str();
    std::vector<float> embedding;
    
    std::istringstream iss(embeddingStr);
    std::string value;
    
    while (std::getline(iss, value, ',')) {
        try {
            embedding.push_back(std::stof(value));
        } catch (const std::exception& e) {
            // Skip invalid values
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    performanceMetrics["api_calls"]++;
    performanceMetrics["total_processing_time"] += duration.count();
    
    return embedding;
}

std::string NeuralEmbeddingEngine::makeHttpRequest(const std::string& url, 
                                                  const std::string& payload,
                                                  const std::map<std::string, std::string>& headers) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    curl = curl_easy_init();
    if (!curl) {
        lastError = "Failed to initialize CURL";
        return "";
    }
    
    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    
    // Set POST data
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    
    // Set headers
    struct curl_slist* headerList = nullptr;
    for (const auto& header : headers) {
        std::string headerStr = header.first + ": " + header.second;
        headerList = curl_slist_append(headerList, headerStr.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    
    // Set callback
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    
    // Set timeout
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    
    // Perform request
    res = curl_easy_perform(curl);
    
    // Check response code
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    
    // Cleanup
    curl_slist_free_all(headerList);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        lastError = "HTTP request failed: " + std::string(curl_easy_strerror(res));
        return "";
    }
    
    if (response_code != 200) {
        lastError = "HTTP error: " + std::to_string(response_code);
        return "";
    }
    
    return readBuffer;
}

TextEmbedding NeuralEmbeddingEngine::embedText(const std::string& text) {
    if (!initialized) {
        lastError = "Engine not initialized";
        return {};
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    TextEmbedding result;
    result.text = text;
    result.modelUsed = config.modelType;
    result.fromCache = false;
    
    // Check cache first
    if (config.enableCaching && isCached(text)) {
        result = getCachedEmbedding(text);
        result.fromCache = true;
        performanceMetrics["cache_hits"]++;
        return result;
    }
    
    performanceMetrics["cache_misses"]++;
    
    // Generate embedding based on model type
    if (config.modelType == "local_onnx") {
        std::vector<int> tokens = tokenizeText(text);
        result.values = runONNXInference(tokens);
    } else if (config.modelType == "openai") {
        result.values = callOpenAIEmbedding(text);
    } else if (config.modelType == "huggingface") {
        result.values = callHuggingFaceEmbedding(text);
    } else if (config.modelType == "deepseek") {
        result.values = callDeepSeekEmbedding(text);
    } else {
        lastError = "Unsupported model type: " + config.modelType;
        return {};
    }
    
    if (result.values.empty()) {
        return {};
    }
    
    result.dimension = result.values.size();
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    result.processingTime = duration.count();
    
    // Cache the result
    if (config.enableCaching) {
        cacheEmbedding(text, result);
    }
    
    performanceMetrics["total_embeddings"]++;
    
    return result;
}

std::vector<float> NeuralEmbeddingEngine::embedTextSimple(const std::string& text) {
    TextEmbedding result = embedText(text);
    return result.values;
}

std::vector<float> NeuralEmbeddingEngine::normalizeEmbedding(const std::vector<float>& embedding) {
    // L2 normalization
    float norm = 0.0f;
    for (float val : embedding) {
        norm += val * val;
    }
    norm = std::sqrt(norm);
    
    if (norm == 0.0f) {
        return embedding;
    }
    
    std::vector<float> normalized;
    normalized.reserve(embedding.size());
    
    for (float val : embedding) {
        normalized.push_back(val / norm);
    }
    
    return normalized;
}

double NeuralEmbeddingEngine::cosineSimilarity(const std::vector<float>& a, const std::vector<float>& b) {
    if (a.size() != b.size()) {
        return 0.0;
    }
    
    double dotProduct = 0.0;
    double normA = 0.0;
    double normB = 0.0;
    
    for (size_t i = 0; i < a.size(); ++i) {
        dotProduct += a[i] * b[i];
        normA += a[i] * a[i];
        normB += b[i] * b[i];
    }
    
    if (normA == 0.0 || normB == 0.0) {
        return 0.0;
    }
    
    return dotProduct / (std::sqrt(normA) * std::sqrt(normB));
}

EmbeddingConfig NeuralEmbeddingEngine::createDefaultConfig() {
    EmbeddingConfig config;
    config.modelPath = "../models/sentence-transformer.onnx";
    config.modelType = "local_onnx";
    config.apiKey = "";
    config.apiEndpoint = "";
    config.maxSequenceLength = 512;
    config.embeddingDimension = 384;  // all-MiniLM-L6-v2 dimension
    config.batchSize = 32;
    config.useGPU = false;
    config.quantized = false;
    config.cacheDirectory = "../cache";
    config.enableCaching = true;
    
    return config;
}

std::string NeuralEmbeddingEngine::getLastError() const {
    return lastError;
}

bool NeuralEmbeddingEngine::isInitialized() const {
    return initialized;
}

