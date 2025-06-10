#ifndef LLM_CLIENT_H
#define LLM_CLIENT_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <chrono>

struct LLMMessage {
    std::string role;        // "system", "user", "assistant"
    std::string content;     // Message content
    std::map<std::string, std::string> metadata;  // Additional metadata
};

struct LLMRequest {
    std::string model;                    // Model identifier
    std::vector<LLMMessage> messages;     // Conversation messages
    int maxTokens;                        // Maximum response tokens
    double temperature;                   // Sampling temperature (0.0-2.0)
    double topP;                          // Nucleus sampling parameter
    int topK;                             // Top-K sampling parameter
    std::vector<std::string> stopSequences;  // Stop generation sequences
    bool stream;                          // Enable streaming response
    std::map<std::string, std::string> metadata;  // Request metadata
};

struct LLMResponse {
    std::string id;                       // Response ID
    std::string model;                    // Model used
    std::string content;                  // Generated content
    std::string finishReason;             // Completion reason
    int promptTokens;                     // Input token count
    int completionTokens;                 // Output token count
    int totalTokens;                      // Total token count
    double processingTime;                // Response time in milliseconds
    std::string provider;                 // API provider used
    bool fromCache;                       // Whether response was cached
    std::map<std::string, std::string> metadata;  // Response metadata
};

struct LLMConfig {
    std::string apiKey;                   // OpenRouter API key
    std::string baseUrl;                  // API base URL
    std::string userAgent;                // User agent string
    std::string appName;                  // Application name for leaderboards
    std::string appUrl;                   // Application URL for leaderboards
    
    // Model preferences (in priority order)
    std::vector<std::string> preferredModels;
    std::string fallbackModel;            // Final fallback model
    
    // Rate limiting
    int maxRequestsPerMinute;             // Rate limit
    int maxConcurrentRequests;            // Concurrent request limit
    double requestDelayMs;                // Delay between requests
    
    // Retry configuration
    int maxRetries;                       // Maximum retry attempts
    double retryDelayMs;                  // Initial retry delay
    double retryBackoffMultiplier;        // Exponential backoff multiplier
    
    // Timeout configuration
    int connectionTimeoutMs;              // Connection timeout
    int requestTimeoutMs;                 // Request timeout
    
    // Caching
    bool enableCaching;                   // Enable response caching
    std::string cacheDirectory;           // Cache storage directory
    int cacheExpiryHours;                 // Cache expiry time
    
    // Logging
    bool enableLogging;                   // Enable request/response logging
    std::string logDirectory;             // Log storage directory
    bool logRequestContent;               // Log full request content
    bool logResponseContent;              // Log full response content
};

class LLMClient {
private:
    LLMConfig config;
    
    // HTTP client state
    void* curlHandle;                     // CURL handle
    std::map<std::string, std::string> defaultHeaders;
    
    // Rate limiting
    std::chrono::steady_clock::time_point lastRequestTime;
    int requestCount;
    std::chrono::steady_clock::time_point rateLimitResetTime;
    
    // Caching system
    std::map<std::string, LLMResponse> responseCache;
    std::string cacheFilePath;
    
    // Performance tracking
    mutable std::map<std::string, double> performanceMetrics;
    
    // Error handling
    mutable std::string lastError;
    std::vector<std::string> errorHistory;
    
    // Model management
    std::vector<std::string> availableModels;
    std::map<std::string, bool> modelHealth;
    std::string currentModel;
    
    // Internal methods
    bool initializeHttpClient();
    void setupDefaultHeaders();
    std::string makeHttpRequest(const std::string& url, 
                               const std::string& payload,
                               const std::map<std::string, std::string>& headers);
    
    // Rate limiting
    bool checkRateLimit();
    void updateRateLimit();
    void waitForRateLimit();
    
    // Model selection and fallback
    std::string selectBestModel(const LLMRequest& request);
    bool isModelHealthy(const std::string& model);
    void markModelUnhealthy(const std::string& model);
    void updateModelHealth();
    
    // Request/response processing
    std::string buildRequestPayload(const LLMRequest& request);
    LLMResponse parseResponse(const std::string& responseBody, 
                             const std::string& model,
                             double processingTime);
    bool validateRequest(const LLMRequest& request);
    bool validateResponse(const LLMResponse& response);
    
    // Caching
    std::string computeRequestHash(const LLMRequest& request);
    bool isCached(const std::string& requestHash);
    LLMResponse getCachedResponse(const std::string& requestHash);
    void cacheResponse(const std::string& requestHash, const LLMResponse& response);
    bool loadResponseCache();
    bool saveResponseCache();
    
    // Retry logic
    LLMResponse executeWithRetry(const LLMRequest& request);
    bool shouldRetry(const std::string& error, int attemptNumber);
    double calculateRetryDelay(int attemptNumber);
    
    // Logging
    void logRequest(const LLMRequest& request, const std::string& model);
    void logResponse(const LLMResponse& response);
    void logError(const std::string& error, const LLMRequest& request);
    
    // Utility methods
    std::string escapeJsonString(const std::string& input);
    std::string getCurrentTimestamp();
    std::string formatDuration(double milliseconds);

public:
    explicit LLMClient(const LLMConfig& config);
    ~LLMClient();
    
    // Initialization and configuration
    bool initialize();
    bool isInitialized() const;
    void updateConfig(const LLMConfig& newConfig);
    LLMConfig getConfig() const;
    
    // Core LLM operations
    LLMResponse generateCompletion(const LLMRequest& request);
    LLMResponse generateCompletion(const std::string& prompt, 
                                  const std::string& model = "");
    
    // Streaming support
    using StreamCallback = std::function<void(const std::string& chunk)>;
    bool generateCompletionStream(const LLMRequest& request, 
                                 StreamCallback callback);
    
    // Conversation management
    LLMResponse continueConversation(std::vector<LLMMessage>& conversation,
                                    const std::string& userMessage,
                                    const std::string& model = "");
    
    // Financial document specific methods
    LLMResponse summarizeDocument(const std::string& documentContent,
                                 const std::string& summaryType = "executive",
                                 int maxSummaryTokens = 1000);
    
    LLMResponse analyzeFinancialRisks(const std::string& documentContent,
                                     const std::vector<std::string>& riskCategories = {});
    
    LLMResponse extractFinancialMetrics(const std::string& documentContent,
                                       const std::vector<std::string>& metricTypes = {});
    
    LLMResponse compareCompanies(const std::vector<std::string>& companyDocuments,
                                const std::vector<std::string>& companyNames,
                                const std::string& comparisonAspect = "financial_performance");
    
    LLMResponse enhanceQuery(const std::string& userQuery,
                            const std::string& context = "",
                            const std::string& domain = "finance");
    
    // Model management
    std::vector<std::string> getAvailableModels();
    bool refreshModelList();
    std::string getCurrentModel() const;
    bool switchModel(const std::string& model);
    std::map<std::string, bool> getModelHealthStatus();
    
    // Performance and diagnostics
    std::map<std::string, double> getPerformanceMetrics() const;
    void resetPerformanceMetrics();
    std::string getSystemInfo() const;
    bool runDiagnostics();
    
    // Cache management
    void clearCache();
    int getCacheSize() const;
    double getCacheHitRate() const;
    bool exportCache(const std::string& filePath);
    bool importCache(const std::string& filePath);
    
    // Error handling
    std::string getLastError() const;
    std::vector<std::string> getErrorHistory() const;
    void clearErrorHistory();
    
    // Utility functions
    bool testConnection();
    std::string getApiStatus();
    double estimateCost(const LLMRequest& request);
    
    // Static utility methods
    static LLMConfig createDefaultConfig();
    static std::vector<std::string> getDefaultModelPriority();
    static bool validateApiKey(const std::string& apiKey);
    static std::string formatPrompt(const std::string& template_str,
                                   const std::map<std::string, std::string>& variables);
};

// Convenience functions for common financial analysis tasks
namespace FinancialLLM {
    // Quick document analysis
    std::string quickSummarize(const std::string& document, 
                              const std::string& apiKey,
                              int maxTokens = 500);
    
    // Risk analysis
    std::vector<std::string> extractRisks(const std::string& document,
                                         const std::string& apiKey,
                                         int maxRisks = 10);
    
    // Financial metrics extraction
    std::map<std::string, std::string> extractMetrics(const std::string& document,
                                                      const std::string& apiKey);
    
    // Company comparison
    std::string compareCompanies(const std::vector<std::string>& documents,
                                const std::vector<std::string>& names,
                                const std::string& apiKey);
    
    // Query enhancement for better retrieval
    std::string enhanceSearchQuery(const std::string& query,
                                  const std::string& apiKey);
}

#endif // LLM_CLIENT_H

