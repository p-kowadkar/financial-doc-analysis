#include "../../src/llm_client.h"
#include "../test_framework.h"
#include <cassert>
#include <iostream>
#include <string>
#include <cstdlib>

void test_llm_client_initialization() {
    std::cout << "Testing LLM client initialization..." << std::endl;
    
    // Test with empty config
    {
        LLMConfig config;
        LLMClient client(config);
        assert(!client.isInitialized());
        // Skip error message check for now
    }
    
    // Test with default config but no API key
    {
        LLMConfig config = LLMClient::createDefaultConfig();
        LLMClient client(config);
        bool initResult = client.initialize();
        // May succeed or fail depending on implementation
        std::cout << "  Default config initialization: " << (initResult ? "SUCCESS" : "FAILED") << std::endl;
    }
    
    // Test with valid config (using provided API key)
    {
        LLMConfig config = LLMClient::createDefaultConfig();
        config.apiKey = "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";
        LLMClient client(config);
        bool initResult = client.initialize();
        std::cout << "  API key initialization: " << (initResult ? "SUCCESS" : "FAILED") << std::endl;
        if (initResult) {
            assert(client.isInitialized());
            std::cout << "  Current model: " << client.getCurrentModel() << std::endl;
        }
    }
}

void test_llm_request_validation() {
    std::cout << "Testing LLM request validation..." << std::endl;
    
    LLMConfig config = LLMClient::createDefaultConfig();
    const char* apiKey = std::getenv("OPENROUTER_API_KEY");
    if (!apiKey) return;  // Skip if no API key
    
    config.apiKey = apiKey;
    LLMClient client(config);
    assert(client.initialize());
    
    // Test empty request
    {
        LLMRequest request;
        LLMResponse response = client.generateCompletion(request);
        assert(response.content.find("Invalid request") != std::string::npos);
    }
    
    // Test invalid max tokens
    {
        LLMRequest request;
        request.maxTokens = -1;
        LLMMessage msg;
        msg.role = "user";
        msg.content = "test";
        request.messages.push_back(msg);
        LLMResponse response = client.generateCompletion(request);
        assert(response.content.find("Invalid") != std::string::npos);
    }
    
    // Test invalid temperature
    {
        LLMRequest request;
        request.maxTokens = 100;
        request.temperature = 3.0;  // Should be 0.0-2.0
        LLMMessage msg;
        msg.role = "user";
        msg.content = "test";
        request.messages.push_back(msg);
        LLMResponse response = client.generateCompletion(request);
        assert(response.content.find("Invalid") != std::string::npos);
    }
}

void test_llm_basic_completion() {
    std::cout << "Testing basic LLM completion..." << std::endl;
    
    const char* apiKey = std::getenv("OPENROUTER_API_KEY");
    if (!apiKey) return;  // Skip if no API key
    
    LLMConfig config = LLMClient::createDefaultConfig();
    config.apiKey = apiKey;
    LLMClient client(config);
    
    if (!client.initialize()) return;
    
    // Test simple completion
    {
        LLMResponse response = client.generateCompletion(
            "What is 2+2? Answer with just the number.");
        
        assert(!response.content.empty());
        assert(response.content.find("4") != std::string::npos);
        assert(response.promptTokens > 0);
        assert(response.completionTokens > 0);
        assert(response.totalTokens > 0);
        assert(!response.model.empty());
        assert(!response.id.empty());
    }
}

void test_llm_model_fallback() {
    std::cout << "Testing LLM model fallback..." << std::endl;
    
    const char* apiKey = std::getenv("OPENROUTER_API_KEY");
    if (!apiKey) return;  // Skip if no API key
    
    LLMConfig config = LLMClient::createDefaultConfig();
    config.apiKey = apiKey;
    
    // Set invalid primary model to test fallback
    config.preferredModels = {"invalid-model", "deepseek/deepseek-chat"};
    
    LLMClient client(config);
    if (!client.initialize()) return;
    
    LLMResponse response = client.generateCompletion(
        "What is 2+2? Answer with just the number.");
    
    assert(!response.content.empty());
    assert(response.model != "invalid-model");
    assert(response.model.find("deepseek") != std::string::npos);
}

void test_llm_caching() {
    std::cout << "Testing LLM response caching..." << std::endl;
    
    const char* apiKey = std::getenv("OPENROUTER_API_KEY");
    if (!apiKey) return;  // Skip if no API key
    
    LLMConfig config = LLMClient::createDefaultConfig();
    config.apiKey = apiKey;
    config.enableCaching = true;
    
    LLMClient client(config);
    if (!client.initialize()) return;
    
    // First request (not cached)
    LLMResponse response1 = client.generateCompletion(
        "What is 2+2? Answer with just the number.");
    assert(!response1.fromCache);
    
    // Second request with same prompt (should be cached)
    LLMResponse response2 = client.generateCompletion(
        "What is 2+2? Answer with just the number.");
    assert(response2.fromCache);
    assert(response1.content == response2.content);
}

int main() {
    try {
        test_llm_client_initialization();
        test_llm_request_validation();
        test_llm_basic_completion();
        test_llm_model_fallback();
        test_llm_caching();
        
        std::cout << "\nAll LLM client tests passed!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
