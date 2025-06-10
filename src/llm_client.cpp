#include "llm_client.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <regex>
#include <cmath>
#include <random>
#include <iomanip>
#include <thread>
#include <curl/curl.h>

// HTTP callback for libcurl
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

LLMClient::LLMClient(const LLMConfig& config) : config(config), curlHandle(nullptr) {
    // Initialize performance metrics
    performanceMetrics["total_requests"] = 0;
    performanceMetrics["successful_requests"] = 0;
    performanceMetrics["failed_requests"] = 0;
    performanceMetrics["total_processing_time"] = 0;
    performanceMetrics["cache_hits"] = 0;
    performanceMetrics["cache_misses"] = 0;
    performanceMetrics["total_tokens_used"] = 0;
    performanceMetrics["total_cost_usd"] = 0;
    
    // Initialize rate limiting
    requestCount = 0;
    lastRequestTime = std::chrono::steady_clock::now();
    rateLimitResetTime = std::chrono::steady_clock::now();
    
    // Set cache file path
    cacheFilePath = config.cacheDirectory + "/llm_response_cache.json";
    
    // Initialize model health tracking
    for (const auto& model : config.preferredModels) {
        modelHealth[model] = true;  // Assume healthy initially
    }
    
    currentModel = config.preferredModels.empty() ? 
                   "deepseek/deepseek-r1-0528" : config.preferredModels[0];
}

LLMClient::~LLMClient() {
    if (config.enableCaching) {
        saveResponseCache();
    }
    
    if (curlHandle) {
        curl_easy_cleanup((CURL*)curlHandle);
    }
}

bool LLMClient::initialize() {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    try {
        // Initialize HTTP client
        if (!initializeHttpClient()) {
            lastError = "Failed to initialize HTTP client";
            return false;
        }
        
        // Setup default headers
        setupDefaultHeaders();
        
        // Load response cache if enabled
        if (config.enableCaching) {
            loadResponseCache();
        }
        
        // Test connection
        if (!testConnection()) {
            lastError = "Failed to connect to OpenRouter API";
            return false;
        }
        
        // Refresh available models
        refreshModelList();
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        performanceMetrics["initialization_time"] = duration.count();
        
        std::cout << "[INFO] LLM client initialized successfully" << std::endl;
        std::cout << "[INFO] Primary model: " << currentModel << std::endl;
        std::cout << "[INFO] Available models: " << availableModels.size() << std::endl;
        std::cout << "[INFO] Initialization time: " << duration.count() << "ms" << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        lastError = "Initialization failed: " + std::string(e.what());
        return false;
    }
}

bool LLMClient::initializeHttpClient() {
    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curlHandle = curl_easy_init();
    
    if (!curlHandle) {
        lastError = "Failed to initialize CURL";
        return false;
    }
    
    // Set common options
    curl_easy_setopt((CURL*)curlHandle, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt((CURL*)curlHandle, CURLOPT_TIMEOUT, config.requestTimeoutMs / 1000);
    curl_easy_setopt((CURL*)curlHandle, CURLOPT_CONNECTTIMEOUT, config.connectionTimeoutMs / 1000);
    curl_easy_setopt((CURL*)curlHandle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt((CURL*)curlHandle, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt((CURL*)curlHandle, CURLOPT_SSL_VERIFYHOST, 2L);
    
    return true;
}

void LLMClient::setupDefaultHeaders() {
    defaultHeaders["Content-Type"] = "application/json";
    defaultHeaders["Authorization"] = "Bearer " + config.apiKey;
    defaultHeaders["User-Agent"] = config.userAgent;
    
    // Optional OpenRouter-specific headers for leaderboards
    if (!config.appName.empty()) {
        defaultHeaders["X-Title"] = config.appName;
    }
    if (!config.appUrl.empty()) {
        defaultHeaders["HTTP-Referer"] = config.appUrl;
    }
}

std::string LLMClient::makeHttpRequest(const std::string& url, 
                                      const std::string& payload,
                                      const std::map<std::string, std::string>& headers) {
    if (!curlHandle) {
        lastError = "HTTP client not initialized";
        return "";
    }
    
    std::string response;
    
    // Set URL
    curl_easy_setopt((CURL*)curlHandle, CURLOPT_URL, url.c_str());
    
    // Set POST data
    curl_easy_setopt((CURL*)curlHandle, CURLOPT_POSTFIELDS, payload.c_str());
    
    // Set headers
    struct curl_slist* headerList = nullptr;
    
    // Add default headers
    for (const auto& header : defaultHeaders) {
        std::string headerStr = header.first + ": " + header.second;
        headerList = curl_slist_append(headerList, headerStr.c_str());
    }
    
    // Add custom headers
    for (const auto& header : headers) {
        std::string headerStr = header.first + ": " + header.second;
        headerList = curl_slist_append(headerList, headerStr.c_str());
    }
    
    curl_easy_setopt((CURL*)curlHandle, CURLOPT_HTTPHEADER, headerList);
    
    // Set callback
    curl_easy_setopt((CURL*)curlHandle, CURLOPT_WRITEDATA, &response);
    
    // Perform request
    CURLcode res = curl_easy_perform((CURL*)curlHandle);
    
    // Check response code
    long responseCode;
    curl_easy_getinfo((CURL*)curlHandle, CURLINFO_RESPONSE_CODE, &responseCode);
    
    // Cleanup
    curl_slist_free_all(headerList);
    
    if (res != CURLE_OK) {
        lastError = "HTTP request failed: " + std::string(curl_easy_strerror(res));
        return "";
    }
    
    if (responseCode != 200) {
        lastError = "HTTP error: " + std::to_string(responseCode) + " - " + response;
        return "";
    }
    
    return response;
}

std::string LLMClient::selectBestModel(const LLMRequest& request) {
    // If specific model requested and healthy, use it
    if (!request.model.empty() && isModelHealthy(request.model)) {
        return request.model;
    }
    
    // Try preferred models in order
    for (const auto& model : config.preferredModels) {
        if (isModelHealthy(model)) {
            return model;
        }
    }
    
    // Fall back to default model
    if (isModelHealthy(config.fallbackModel)) {
        return config.fallbackModel;
    }
    
    // Last resort - use current model even if unhealthy
    return currentModel;
}

bool LLMClient::isModelHealthy(const std::string& model) {
    auto it = modelHealth.find(model);
    return it != modelHealth.end() ? it->second : true;
}

std::string LLMClient::buildRequestPayload(const LLMRequest& request) {
    std::ostringstream payload;
    payload << "{";
    
    // Model
    payload << "\"model\":\"" << escapeJsonString(request.model) << "\",";
    
    // Messages
    payload << "\"messages\":[";
    for (size_t i = 0; i < request.messages.size(); ++i) {
        if (i > 0) payload << ",";
        payload << "{";
        payload << "\"role\":\"" << escapeJsonString(request.messages[i].role) << "\",";
        payload << "\"content\":\"" << escapeJsonString(request.messages[i].content) << "\"";
        payload << "}";
    }
    payload << "],";
    
    // Parameters
    payload << "\"max_tokens\":" << request.maxTokens << ",";
    payload << "\"temperature\":" << request.temperature << ",";
    payload << "\"top_p\":" << request.topP;
    
    if (request.topK > 0) {
        payload << ",\"top_k\":" << request.topK;
    }
    
    if (!request.stopSequences.empty()) {
        payload << ",\"stop\":[";
        for (size_t i = 0; i < request.stopSequences.size(); ++i) {
            if (i > 0) payload << ",";
            payload << "\"" << escapeJsonString(request.stopSequences[i]) << "\"";
        }
        payload << "]";
    }
    
    if (request.stream) {
        payload << ",\"stream\":true";
    }
    
    payload << "}";
    
    return payload.str();
}

LLMResponse LLMClient::parseResponse(const std::string& responseBody, 
                                    const std::string& model,
                                    double processingTime) {
    LLMResponse response;
    response.model = model;
    response.processingTime = processingTime;
    response.provider = "openrouter";
    response.fromCache = false;
    
    try {
        // Simple JSON parsing using regex (for production, use proper JSON parser)
        std::regex idPattern("\"id\":\\s*\"([^\"]+)\"");
        std::regex contentPattern("\"content\":\\s*\"([^\"]*(?:\\\\.[^\"]*)*)\",?");
        std::regex finishReasonPattern("\"finish_reason\":\\s*\"([^\"]+)\"");
        std::regex promptTokensPattern(R"("prompt_tokens":\s*(\d+))");
        std::regex completionTokensPattern(R"("completion_tokens":\s*(\d+))");
        std::regex totalTokensPattern(R"("total_tokens":\s*(\d+))");
        
        std::smatch match;
        
        // Extract ID
        if (std::regex_search(responseBody, match, idPattern)) {
            response.id = match[1].str();
        }
        
        // Extract content
        if (std::regex_search(responseBody, match, contentPattern)) {
            response.content = match[1].str();
            // Unescape JSON string
            std::regex escapePattern(R"(\\(.))");
            response.content = std::regex_replace(response.content, escapePattern, "$1");
        }
        
        // Extract finish reason
        if (std::regex_search(responseBody, match, finishReasonPattern)) {
            response.finishReason = match[1].str();
        }
        
        // Extract token counts
        if (std::regex_search(responseBody, match, promptTokensPattern)) {
            response.promptTokens = std::stoi(match[1].str());
        }
        
        if (std::regex_search(responseBody, match, completionTokensPattern)) {
            response.completionTokens = std::stoi(match[1].str());
        }
        
        if (std::regex_search(responseBody, match, totalTokensPattern)) {
            response.totalTokens = std::stoi(match[1].str());
        }
        
    } catch (const std::exception& e) {
        lastError = "Failed to parse response: " + std::string(e.what());
        response.content = "Error parsing response";
    }
    
    return response;
}

LLMResponse LLMClient::generateCompletion(const LLMRequest& request) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    performanceMetrics["total_requests"]++;
    
    // Validate request
    if (!validateRequest(request)) {
        LLMResponse errorResponse;
        errorResponse.content = "Invalid request: " + lastError;
        performanceMetrics["failed_requests"]++;
        return errorResponse;
    }
    
    // Check cache first
    std::string requestHash = computeRequestHash(request);
    if (config.enableCaching && isCached(requestHash)) {
        LLMResponse cachedResponse = getCachedResponse(requestHash);
        cachedResponse.fromCache = true;
        performanceMetrics["cache_hits"]++;
        return cachedResponse;
    }
    
    performanceMetrics["cache_misses"]++;
    
    // Select best model
    LLMRequest modifiedRequest = request;
    modifiedRequest.model = selectBestModel(request);
    currentModel = modifiedRequest.model;
    
    // Execute with retry logic
    LLMResponse response = executeWithRetry(modifiedRequest);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    response.processingTime = duration.count();
    
    // Update metrics
    if (!response.content.empty() && response.content != "Error parsing response") {
        performanceMetrics["successful_requests"]++;
        performanceMetrics["total_tokens_used"] += response.totalTokens;
    } else {
        performanceMetrics["failed_requests"]++;
    }
    
    performanceMetrics["total_processing_time"] += duration.count();
    
    // Cache successful response
    if (config.enableCaching && !response.content.empty()) {
        cacheResponse(requestHash, response);
    }
    
    // Log request/response if enabled
    if (config.enableLogging) {
        logRequest(modifiedRequest, modifiedRequest.model);
        logResponse(response);
    }
    
    return response;
}

LLMResponse LLMClient::executeWithRetry(const LLMRequest& request) {
    LLMResponse response;
    
    for (int attempt = 0; attempt < config.maxRetries; ++attempt) {
        // Check rate limit
        if (!checkRateLimit()) {
            waitForRateLimit();
        }
        
        // Build payload
        std::string payload = buildRequestPayload(request);
        
        // Make request
        auto requestStart = std::chrono::high_resolution_clock::now();
        std::string responseBody = makeHttpRequest(config.baseUrl + "/chat/completions", 
                                                  payload, {});
        auto requestEnd = std::chrono::high_resolution_clock::now();
        
        updateRateLimit();
        
        if (!responseBody.empty()) {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                requestEnd - requestStart);
            response = parseResponse(responseBody, request.model, duration.count());
            
            if (validateResponse(response)) {
                return response;
            }
        }
        
        // Mark model as unhealthy if multiple failures
        if (attempt >= 2) {
            markModelUnhealthy(request.model);
        }
        
        // Calculate retry delay
        if (attempt < config.maxRetries - 1) {
            double delay = calculateRetryDelay(attempt);
            std::this_thread::sleep_for(std::chrono::milliseconds((int)delay));
        }
        
        // Log error
        if (config.enableLogging) {
            logError(lastError, request);
        }
    }
    
    // All retries failed
    response.content = "Request failed after " + std::to_string(config.maxRetries) + " attempts";
    return response;
}

// Financial document specific methods
LLMResponse LLMClient::summarizeDocument(const std::string& documentContent,
                                        const std::string& summaryType,
                                        int maxSummaryTokens) {
    LLMRequest request;
    request.model = currentModel;
    request.maxTokens = maxSummaryTokens;
    request.temperature = 0.1;  // Low temperature for factual summaries
    request.topP = 0.9;
    
    // System message for financial document summarization
    LLMMessage systemMsg;
    systemMsg.role = "system";
    systemMsg.content = "You are a financial analyst expert. Provide accurate, concise summaries of financial documents. Focus on key financial metrics, risks, and business insights.";
    
    // User message with document content
    LLMMessage userMsg;
    userMsg.role = "user";
    
    if (summaryType == "executive") {
        userMsg.content = "Provide an executive summary of this financial document, highlighting key financial performance, major risks, and strategic initiatives:\n\n" + documentContent;
    } else if (summaryType == "risks") {
        userMsg.content = "Analyze and summarize the key financial and business risks mentioned in this document:\n\n" + documentContent;
    } else if (summaryType == "metrics") {
        userMsg.content = "Extract and summarize the key financial metrics and performance indicators from this document:\n\n" + documentContent;
    } else {
        userMsg.content = "Provide a comprehensive summary of this financial document:\n\n" + documentContent;
    }
    
    request.messages = {systemMsg, userMsg};
    
    return generateCompletion(request);
}

LLMResponse LLMClient::analyzeFinancialRisks(const std::string& documentContent,
                                            const std::vector<std::string>& riskCategories) {
    LLMRequest request;
    request.model = currentModel;
    request.maxTokens = 2000;
    request.temperature = 0.1;
    request.topP = 0.9;
    
    LLMMessage systemMsg;
    systemMsg.role = "system";
    systemMsg.content = "You are a risk analysis expert specializing in financial documents. Identify, categorize, and assess risks with specific focus on financial impact and likelihood.";
    
    LLMMessage userMsg;
    userMsg.role = "user";
    
    std::string prompt = "Analyze the financial and business risks in this document. ";
    
    if (!riskCategories.empty()) {
        prompt += "Focus specifically on these risk categories: ";
        for (size_t i = 0; i < riskCategories.size(); ++i) {
            if (i > 0) prompt += ", ";
            prompt += riskCategories[i];
        }
        prompt += ". ";
    }
    
    prompt += "For each risk identified, provide:\n";
    prompt += "1. Risk category\n";
    prompt += "2. Description\n";
    prompt += "3. Potential financial impact\n";
    prompt += "4. Likelihood assessment\n";
    prompt += "5. Mitigation strategies mentioned\n\n";
    prompt += "Document content:\n" + documentContent;
    
    userMsg.content = prompt;
    request.messages = {systemMsg, userMsg};
    
    return generateCompletion(request);
}

LLMResponse LLMClient::extractFinancialMetrics(const std::string& documentContent,
                                              const std::vector<std::string>& metricTypes) {
    LLMRequest request;
    request.model = currentModel;
    request.maxTokens = 1500;
    request.temperature = 0.0;  // Very low temperature for precise extraction
    request.topP = 0.8;
    
    LLMMessage systemMsg;
    systemMsg.role = "system";
    systemMsg.content = "You are a financial data extraction expert. Extract precise financial metrics and numbers from documents. Always include the specific values, units, and time periods.";
    
    LLMMessage userMsg;
    userMsg.role = "user";
    
    std::string prompt = "Extract financial metrics from this document. ";
    
    if (!metricTypes.empty()) {
        prompt += "Focus on these specific metrics: ";
        for (size_t i = 0; i < metricTypes.size(); ++i) {
            if (i > 0) prompt += ", ";
            prompt += metricTypes[i];
        }
        prompt += ". ";
    } else {
        prompt += "Include revenue, profit margins, cash flow, debt levels, growth rates, and other key financial indicators. ";
    }
    
    prompt += "Format the output as:\n";
    prompt += "Metric Name: Value (Unit) [Time Period]\n\n";
    prompt += "Document content:\n" + documentContent;
    
    userMsg.content = prompt;
    request.messages = {systemMsg, userMsg};
    
    return generateCompletion(request);
}

LLMResponse LLMClient::enhanceQuery(const std::string& userQuery,
                                   const std::string& context,
                                   const std::string& domain) {
    LLMRequest request;
    request.model = currentModel;
    request.maxTokens = 300;
    request.temperature = 0.3;
    request.topP = 0.9;
    
    LLMMessage systemMsg;
    systemMsg.role = "system";
    systemMsg.content = "You are a query enhancement expert. Improve search queries to be more specific and effective for document retrieval in the " + domain + " domain.";
    
    LLMMessage userMsg;
    userMsg.role = "user";
    userMsg.content = "Enhance this search query to be more specific and effective for finding relevant information:\n\n";
    userMsg.content += "Original query: " + userQuery + "\n";
    
    if (!context.empty()) {
        userMsg.content += "Context: " + context + "\n";
    }
    
    userMsg.content += "\nProvide an enhanced query that includes relevant synonyms, specific terminology, and related concepts that would improve search results.";
    
    request.messages = {systemMsg, userMsg};
    
    return generateCompletion(request);
}

// Utility methods
std::string LLMClient::escapeJsonString(const std::string& input) {
    std::string output;
    output.reserve(input.length() * 2);
    
    for (char c : input) {
        switch (c) {
            case '"': output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\b': output += "\\b"; break;
            case '\f': output += "\\f"; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default:
                if (c < 0x20) {
                    output += "\\u";
                    output += "0000";
                    std::ostringstream oss;
                    oss << std::hex << (int)c;
                    std::string hex = oss.str();
                    output.replace(output.length() - hex.length(), hex.length(), hex);
                } else {
                    output += c;
                }
                break;
        }
    }
    
    return output;
}

bool LLMClient::validateRequest(const LLMRequest& request) {
    if (request.messages.empty()) {
        lastError = "Request must contain at least one message";
        return false;
    }
    
    if (request.maxTokens <= 0 || request.maxTokens > 32000) {
        lastError = "Invalid max_tokens value";
        return false;
    }
    
    if (request.temperature < 0.0 || request.temperature > 2.0) {
        lastError = "Temperature must be between 0.0 and 2.0";
        return false;
    }
    
    return true;
}

bool LLMClient::validateResponse(const LLMResponse& response) {
    return !response.content.empty() && response.content != "Error parsing response";
}

std::string LLMClient::computeRequestHash(const LLMRequest& request) {
    // Simple hash based on model and message content
    std::string hashInput = request.model;
    for (const auto& msg : request.messages) {
        hashInput += msg.role + msg.content;
    }
    hashInput += std::to_string(request.maxTokens);
    hashInput += std::to_string(request.temperature);
    
    // Simple hash function (for production, use proper hash like SHA-256)
    std::hash<std::string> hasher;
    return std::to_string(hasher(hashInput));
}

bool LLMClient::testConnection() {
    try {
        std::string response = makeHttpRequest(config.baseUrl + "/models", "", {});
        return !response.empty();
    } catch (const std::exception& e) {
        lastError = "Connection test failed: " + std::string(e.what());
        return false;
    }
}

LLMConfig LLMClient::createDefaultConfig() {
    LLMConfig config;
    
    config.apiKey = "";  // Must be set by user
    config.baseUrl = "https://openrouter.ai/api/v1";
    config.userAgent = "FinancialAnalysisSystem/1.0";
    config.appName = "C++ Financial Document Analysis System";
    config.appUrl = "https://github.com/financial-analysis";
    
    // Model preferences (DeepSeek first, then OpenAI fallback)
    config.preferredModels = {
        "deepseek/deepseek-r1-0528",           // Free DeepSeek R1
        "deepseek/deepseek-r1-distill-qwen-7b", // Low-cost DeepSeek
        "openai/gpt-4.1-mini"                   // OpenAI fallback
    };
    config.fallbackModel = "openai/gpt-4.1-mini";
    
    // Rate limiting (conservative for free models)
    config.maxRequestsPerMinute = 20;
    config.maxConcurrentRequests = 2;
    config.requestDelayMs = 3000;  // 3 seconds between requests
    
    // Retry configuration
    config.maxRetries = 3;
    config.retryDelayMs = 1000;
    config.retryBackoffMultiplier = 2.0;
    
    // Timeout configuration
    config.connectionTimeoutMs = 10000;  // 10 seconds
    config.requestTimeoutMs = 60000;     // 60 seconds
    
    // Caching
    config.enableCaching = true;
    config.cacheDirectory = "../cache";
    config.cacheExpiryHours = 24;
    
    // Logging
    config.enableLogging = true;
    config.logDirectory = "../logs";
    config.logRequestContent = false;  // Privacy-conscious default
    config.logResponseContent = false;
    
    return config;
}

std::string LLMClient::getLastError() const {
    return lastError;
}

bool LLMClient::isInitialized() const {
    return curlHandle != nullptr;
}

std::string LLMClient::getCurrentModel() const {
    return currentModel;
}

LLMResponse LLMClient::generateCompletion(const std::string& prompt, const std::string& model) {
    LLMRequest request;
    request.maxTokens = 1000;  // Default value
    request.temperature = 0.1; // Default value
    request.model = model.empty() ? currentModel : model;
    LLMMessage userMsg{"user", prompt};
    request.messages = {userMsg};
    return generateCompletion(request);
}

bool LLMClient::saveResponseCache() {
    if (!config.enableCaching) return true;
    // Implementation would go here
    return true;
}

bool LLMClient::loadResponseCache() {
    if (!config.enableCaching) return true;
    // Implementation would go here
    return true;
}

bool LLMClient::refreshModelList() {
    // Implementation would go here
    return true;
}

bool LLMClient::isCached(const std::string& requestHash) {
    if (!config.enableCaching) return false;
    return responseCache.find(requestHash) != responseCache.end();
}

LLMResponse LLMClient::getCachedResponse(const std::string& requestHash) {
    if (!config.enableCaching || !isCached(requestHash)) {
        return LLMResponse{};
    }
    return responseCache.at(requestHash);
}

void LLMClient::cacheResponse(const std::string& key, const LLMResponse& response) {
    if (!config.enableCaching) return;
    responseCache[key] = response;
}

void LLMClient::logRequest(const LLMRequest& request, const std::string& model) {
    if (!config.enableLogging) return;
    // Implementation would go here
}

void LLMClient::logResponse(const LLMResponse& response) {
    if (!config.enableLogging) return;
    // Implementation would go here
}

bool LLMClient::checkRateLimit() {
    // Simple rate limiting implementation
    if (lastRequestTime.time_since_epoch().count() == 0) return true;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRequestTime);
    return elapsed.count() >= config.requestDelayMs;
}

void LLMClient::waitForRateLimit() {
    if (checkRateLimit()) return;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRequestTime);
    auto waitTime = config.requestDelayMs - elapsed.count();
    if (waitTime > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(waitTime)));
    }
}

void LLMClient::updateRateLimit() {
    lastRequestTime = std::chrono::steady_clock::now();
}

void LLMClient::markModelUnhealthy(const std::string& model) {
    modelHealth[model] = false;
}

double LLMClient::calculateRetryDelay(int attemptNumber) {
    // Exponential backoff with jitter
    double baseDelay = 1000.0;  // 1 second
    double maxDelay = 32000.0;  // 32 seconds
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    double delay = baseDelay * std::pow(2.0, attemptNumber);
    delay = std::min(delay, maxDelay);
    delay += delay * 0.1 * dis(gen);  // Add up to 10% jitter
    
    return delay;
}

void LLMClient::logError(const std::string& error, const LLMRequest& request) {
    if (!config.enableLogging) return;
    // Implementation would go here
}

