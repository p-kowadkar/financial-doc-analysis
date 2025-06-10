#include <iostream>
#include <string>
#include <vector>
#include <map>

// Mock LLM client for testing
class LLMClient {
public:
    struct LLMConfig {
        std::string apiKey;
        std::string baseUrl;
        std::vector<std::string> preferredModels;
        int maxTokens = 1000;
        double temperature = 0.7;
    };
    
    struct LLMResponse {
        std::string content;
        std::string model;
        int tokensUsed = 0;
        bool success = false;
        std::string error;
    };
    
    LLMClient(const LLMConfig& config) : config_(config) {}
    
    LLMResponse query(const std::string& prompt, const std::string& context = "") {
        LLMResponse response;
        
        if (prompt.empty()) {
            response.error = "Empty prompt";
            return response;
        }
        
        if (config_.apiKey.empty()) {
            response.error = "No API key configured";
            return response;
        }
        
        // Mock response generation
        response.content = generateMockResponse(prompt, context);
        response.model = config_.preferredModels.empty() ? "mock-model" : config_.preferredModels[0];
        response.tokensUsed = prompt.length() / 4; // Rough estimate
        response.success = true;
        
        return response;
    }
    
    bool validateConfig() const {
        return !config_.apiKey.empty() && !config_.baseUrl.empty();
    }
    
    std::vector<std::string> getAvailableModels() const {
        return {"gpt-3.5-turbo", "gpt-4", "deepseek-chat", "claude-3"};
    }
    
private:
    LLMConfig config_;
    
    std::string generateMockResponse(const std::string& prompt, const std::string& context) {
        // Simple mock response based on prompt content
        if (prompt.find("revenue") != std::string::npos) {
            return "Based on the financial data, revenue shows positive growth trends.";
        }
        if (prompt.find("risk") != std::string::npos) {
            return "Key risk factors include market volatility and regulatory changes.";
        }
        if (prompt.find("summary") != std::string::npos) {
            return "Summary: The company demonstrates strong financial performance.";
        }
        
        return "This is a mock response to: " + prompt.substr(0, 50) + "...";
    }
};

// Test functions
bool testLLMClientBasicFunctionality() {
    LLMClient::LLMConfig config;
    config.apiKey = "test-api-key";
    config.baseUrl = "https://api.test.com";
    config.preferredModels = {"gpt-3.5-turbo"};
    
    LLMClient client(config);
    
    auto response = client.query("What is the revenue growth?");
    
    if (!response.success) {
        std::cout << "FAIL: Basic query failed: " << response.error << std::endl;
        return false;
    }
    
    if (response.content.empty()) {
        std::cout << "FAIL: Empty response content" << std::endl;
        return false;
    }
    
    if (response.tokensUsed <= 0) {
        std::cout << "FAIL: Invalid token count" << std::endl;
        return false;
    }
    
    std::cout << "PASS: Basic functionality test" << std::endl;
    return true;
}

bool testLLMClientConfigValidation() {
    // Test valid config
    LLMClient::LLMConfig validConfig;
    validConfig.apiKey = "test-key";
    validConfig.baseUrl = "https://api.test.com";
    
    LLMClient validClient(validConfig);
    if (!validClient.validateConfig()) {
        std::cout << "FAIL: Valid config should pass validation" << std::endl;
        return false;
    }
    
    // Test invalid config
    LLMClient::LLMConfig invalidConfig;
    // Missing API key and base URL
    
    LLMClient invalidClient(invalidConfig);
    if (invalidClient.validateConfig()) {
        std::cout << "FAIL: Invalid config should fail validation" << std::endl;
        return false;
    }
    
    std::cout << "PASS: Config validation test" << std::endl;
    return true;
}

bool testLLMClientErrorHandling() {
    LLMClient::LLMConfig config;
    config.apiKey = "test-key";
    config.baseUrl = "https://api.test.com";
    
    LLMClient client(config);
    
    // Test empty prompt
    auto response1 = client.query("");
    if (response1.success) {
        std::cout << "FAIL: Empty prompt should fail" << std::endl;
        return false;
    }
    
    // Test with missing API key
    LLMClient::LLMConfig badConfig;
    badConfig.baseUrl = "https://api.test.com";
    // No API key
    
    LLMClient badClient(badConfig);
    auto response2 = badClient.query("test prompt");
    if (response2.success) {
        std::cout << "FAIL: Missing API key should fail" << std::endl;
        return false;
    }
    
    std::cout << "PASS: Error handling test" << std::endl;
    return true;
}

bool testLLMClientContextHandling() {
    LLMClient::LLMConfig config;
    config.apiKey = "test-key";
    config.baseUrl = "https://api.test.com";
    
    LLMClient client(config);
    
    std::string prompt = "Analyze the financial performance";
    std::string context = "Company revenue: $100M, Profit: $20M";
    
    auto response = client.query(prompt, context);
    
    if (!response.success) {
        std::cout << "FAIL: Context query failed: " << response.error << std::endl;
        return false;
    }
    
    if (response.content.empty()) {
        std::cout << "FAIL: Empty response with context" << std::endl;
        return false;
    }
    
    std::cout << "PASS: Context handling test" << std::endl;
    return true;
}

bool testLLMClientModelSelection() {
    LLMClient::LLMConfig config;
    config.apiKey = "test-key";
    config.baseUrl = "https://api.test.com";
    config.preferredModels = {"gpt-4", "gpt-3.5-turbo"};
    
    LLMClient client(config);
    
    auto availableModels = client.getAvailableModels();
    if (availableModels.empty()) {
        std::cout << "FAIL: No available models returned" << std::endl;
        return false;
    }
    
    auto response = client.query("test prompt");
    if (!response.success) {
        std::cout << "FAIL: Model selection query failed" << std::endl;
        return false;
    }
    
    if (response.model.empty()) {
        std::cout << "FAIL: No model specified in response" << std::endl;
        return false;
    }
    
    std::cout << "PASS: Model selection test" << std::endl;
    return true;
}

bool testLLMClientResponseTypes() {
    LLMClient::LLMConfig config;
    config.apiKey = "test-key";
    config.baseUrl = "https://api.test.com";
    
    LLMClient client(config);
    
    // Test different prompt types
    auto revenueResponse = client.query("What is the revenue growth?");
    auto riskResponse = client.query("What are the main risk factors?");
    auto summaryResponse = client.query("Provide a summary of the document");
    
    if (!revenueResponse.success || !riskResponse.success || !summaryResponse.success) {
        std::cout << "FAIL: One or more response types failed" << std::endl;
        return false;
    }
    
    // Check that responses are different and relevant
    if (revenueResponse.content == riskResponse.content) {
        std::cout << "FAIL: Different prompts should generate different responses" << std::endl;
        return false;
    }
    
    std::cout << "PASS: Response types test" << std::endl;
    return true;
}

// Main test runner
int main() {
    std::cout << "Running LLM Client Unit Tests" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    int passed = 0;
    int total = 0;
    
    total++; if (testLLMClientBasicFunctionality()) passed++;
    total++; if (testLLMClientConfigValidation()) passed++;
    total++; if (testLLMClientErrorHandling()) passed++;
    total++; if (testLLMClientContextHandling()) passed++;
    total++; if (testLLMClientModelSelection()) passed++;
    total++; if (testLLMClientResponseTypes()) passed++;
    
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
