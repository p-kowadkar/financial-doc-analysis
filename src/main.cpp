#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdlib>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>
#include "llm_client.h"

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

class FinancialDocumentAnalysisSystem {
private:
    Logger logger;
    std::unique_ptr<LLMClient> llmClient;

    int executeCommand(const std::string& command) {
        return std::system(command.c_str());
    }

    std::string generateTimestampedFilename(const std::string& prefix, const std::string& extension) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ss;
        ss << prefix << "_" << time << "." << extension;
        return ss.str();
    }

    // Initialize LLM client with configuration
    bool initializeLLMClient() {
        try {
            LLMConfig config = LLMClient::createDefaultConfig();
            
            // Try to get API key from environment variable
            const char* apiKey = std::getenv("OPENROUTER_API_KEY");
            if (apiKey) {
                config.apiKey = apiKey;
            } else {
                // Try to read from config file
                std::ifstream configFile("../config/llm_config.conf");
                if (configFile.is_open()) {
                    std::string line;
                    while (std::getline(configFile, line)) {
                        if (line.find("api_key=") == 0) {
                            config.apiKey = line.substr(8);
                            break;
                        }
                    }
                    configFile.close();
                }
            }
            
            if (config.apiKey.empty()) {
                logger.log("ERROR", "OpenRouter API key not found. Set OPENROUTER_API_KEY environment variable or add api_key= to ../config/llm_config.conf");
                return false;
            }
            
            llmClient = std::make_unique<LLMClient>(config);
            
            if (!llmClient->initialize()) {
                logger.log("ERROR", "Failed to initialize LLM client: " + llmClient->getLastError());
                return false;
            }
            
            logger.log("INFO", "LLM client initialized successfully with model: " + llmClient->getCurrentModel());
            return true;
            
        } catch (const std::exception& e) {
            logger.log("ERROR", "Exception during LLM client initialization: " + std::string(e.what()));
            return false;
        }
    }

    // Read file content for LLM processing
    std::string readFileContent(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            logger.log("ERROR", "Cannot open file: " + filePath);
            return "";
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

public:
    FinancialDocumentAnalysisSystem() {
        // Initialize LLM client lazily when needed
    }

    void showHelp() {
        std::cout << "C++ Intelligent Financial Document Analysis System - Windows Compatible" << std::endl;
        std::cout << "========================================================================" << std::endl;
        std::cout << std::endl;
        std::cout << "USAGE:" << std::endl;
        std::cout << "  " << "main.exe" << " <command> [options]" << std::endl;
        std::cout << std::endl;
        std::cout << "COMMANDS:" << std::endl;
        std::cout << "  build-index                    Build document index from structured data" << std::endl;
        std::cout << "  query <query> [options]        Search documents and retrieve results" << std::endl;
        std::cout << "  analyze                        Perform financial analysis on documents" << std::endl;
        std::cout << "  llm <prompt> [options]         Send prompt to LLM for analysis" << std::endl;
        std::cout << "  help                          Show this help message" << std::endl;
        std::cout << std::endl;
        std::cout << "QUERY OPTIONS:" << std::endl;
        std::cout << "  --top <number>                Number of top results to return (default: 3)" << std::endl;
        std::cout << "  --summary                     Generate extractive summary using RAG" << std::endl;
        std::cout << "  --json                        Output results in JSON format" << std::endl;
        std::cout << std::endl;
        std::cout << "LLM OPTIONS:" << std::endl;
        std::cout << "  --file <path>                 Process file content with LLM" << std::endl;
        std::cout << "  --model <model>               Specify model (deepseek-chat, deepseek-r1, gpt-4)" << std::endl;
        std::cout << "  --max-tokens <number>         Maximum response tokens (default: 2000)" << std::endl;
        std::cout << "  --temperature <float>         Sampling temperature 0.0-2.0 (default: 0.1)" << std::endl;
        std::cout << "  --output <path>               Save LLM response to file" << std::endl;
        std::cout << std::endl;
        std::cout << "EXAMPLES:" << std::endl;
        std::cout << "  main.exe build-index" << std::endl;
        std::cout << "  main.exe query \"cybersecurity risks\" --top 3 --summary" << std::endl;
        std::cout << "  main.exe query \"revenue growth\" --top 5 --json" << std::endl;
        std::cout << "  main.exe analyze" << std::endl;
        std::cout << "  main.exe llm \"Summarize risk factors in Apple 10-K\"" << std::endl;
        std::cout << "  main.exe llm \"Analyze financial metrics\" --file ../data/apple_10k_2023_sample.txt" << std::endl;
        std::cout << "  main.exe llm \"Compare revenue trends\" --model deepseek-r1 --max-tokens 1500" << std::endl;
        std::cout << std::endl;
        std::cout << "API SETUP:" << std::endl;
        std::cout << "  Set OPENROUTER_API_KEY environment variable or create ../config/llm_config.conf" << std::endl;
        std::cout << "  Get free API key at: https://openrouter.ai/" << std::endl;
        std::cout << std::endl;
        std::cout << "SYSTEM REQUIREMENTS:" << std::endl;
        std::cout << "  - Windows with C++17 compiler (g++, MSVC, or clang++)" << std::endl;
        std::cout << "  - libcurl for LLM functionality" << std::endl;
        std::cout << "  - Sample data files in ../data/ directory" << std::endl;
        std::cout << std::endl;
    }

    int buildIndex() {
        logger.log("INFO", "Starting document index building process");
        
        // Step 1: Parse documents
        logger.log("INFO", "Parsing Apple 10-K document");
        int result1 = executeCommand("document_parser.exe ../data/apple_10k_2023_sample.txt ../output/apple_10k_2023_structured.json");
        
        logger.log("INFO", "Parsing Microsoft 10-K document");
        int result2 = executeCommand("document_parser.exe ../data/microsoft_10k_2024_sample.txt ../output/microsoft_10k_2024_structured.json");
        
        if (result1 != 0 || result2 != 0) {
            logger.log("ERROR", "Document parsing failed");
            return 1;
        }
        
        // Step 2: Build TF-IDF index
        logger.log("INFO", "Building TF-IDF index");
        int result3 = executeCommand("tfidf_embedding.exe build ../output ../output/financial_docs_index.json");
        
        if (result3 != 0) {
            logger.log("ERROR", "Index building failed");
            return 1;
        }
        
        logger.log("INFO", "Document index built successfully");
        return 0;
    }

    int executeQuery(const std::string& query, int topK, bool useSummary, bool useJSON) {
        logger.log("INFO", "Executing query: \"" + query + "\"");
        
        std::string outputFile;
        std::string command;
        
        if (useSummary) {
            // Use RAG engine for summarized results
            outputFile = "../output/" + generateTimestampedFilename("rag_result", "md");
            command = "rag_engine.exe ../output/financial_docs_index.json \"" + query + "\" " + outputFile + " " + std::to_string(topK);
        } else {
            // Use query engine for direct results
            if (useJSON) {
                outputFile = "../output/" + generateTimestampedFilename("query_result", "json");
                command = "query_engine.exe ../output/financial_docs_index.json \"" + query + "\" " + outputFile + " " + std::to_string(topK) + " --json";
            } else {
                outputFile = "../output/" + generateTimestampedFilename("query_result", "md");
                command = "query_engine.exe ../output/financial_docs_index.json \"" + query + "\" " + outputFile + " " + std::to_string(topK);
            }
        }
        
        int result = executeCommand(command);
        
        if (result == 0) {
            logger.log("INFO", "Query executed successfully");
            logger.log("INFO", "Results saved to: " + outputFile);
            std::cout << std::endl << "Results saved to: " << outputFile << std::endl;
        } else {
            logger.log("ERROR", "Query execution failed");
        }
        
        return result;
    }

    int performAnalysis() {
        logger.log("INFO", "Starting financial analysis");
        
        std::string jsonOutput = "../output/" + generateTimestampedFilename("analysis", "json");
        std::string markdownOutput = "../output/" + generateTimestampedFilename("analysis", "md");
        
        std::string command = "financial_analyzer.exe ../output/financial_docs_index.json " + jsonOutput + " " + markdownOutput;
        
        int result = executeCommand(command);
        
        if (result == 0) {
            logger.log("INFO", "Financial analysis completed successfully");
            logger.log("INFO", "JSON insights saved to: " + jsonOutput);
            logger.log("INFO", "Markdown report saved to: " + markdownOutput);
            std::cout << std::endl << "Analysis completed!" << std::endl;
            std::cout << "JSON insights: " << jsonOutput << std::endl;
            std::cout << "Markdown report: " << markdownOutput << std::endl;
        } else {
            logger.log("ERROR", "Financial analysis failed");
        }
        
        return result;
    }

    int executeLLM(const std::string& prompt, const std::map<std::string, std::string>& options) {
        // Initialize LLM client if not already initialized
        if (!llmClient && !initializeLLMClient()) {
            return 1;
        }

        try {
            // Prepare LLM request
            LLMRequest request;
            
            // Set model if specified
            if (options.count("model")) {
                request.model = options.at("model");
            }
            
            // Set max tokens
            request.maxTokens = options.count("max-tokens") ? 
                std::stoi(options.at("max-tokens")) : 2000;
            
            // Set temperature
            request.temperature = options.count("temperature") ? 
                std::stof(options.at("temperature")) : 0.1;
            
            // Prepare content
            std::string content = prompt;
            
            // If file is specified, read and append its content
            if (options.count("file")) {
                std::string fileContent = readFileContent(options.at("file"));
                if (fileContent.empty()) {
                    return 1;
                }
                content += "\n\nDocument content:\n" + fileContent;
            }
            
            // Set up messages
            LLMMessage systemMsg;
            systemMsg.role = "system";
            systemMsg.content = "You are a financial analysis expert. Analyze documents and answer queries with specific focus on financial metrics, risks, and business insights. Be concise and factual.";
            
            LLMMessage userMsg;
            userMsg.role = "user";
            userMsg.content = content;
            
            request.messages = {systemMsg, userMsg};
            
            // Generate completion
            logger.log("INFO", "Sending request to LLM...");
            LLMResponse response = llmClient->generateCompletion(request);
            
            if (response.content.empty()) {
                logger.log("ERROR", "LLM returned empty response: " + llmClient->getLastError());
                return 1;
            }
            
            // Save to file if output path specified
            if (options.count("output")) {
                std::ofstream outFile(options.at("output"));
                if (outFile.is_open()) {
                    outFile << response.content;
                    outFile.close();
                    logger.log("INFO", "Response saved to: " + options.at("output"));
                } else {
                    logger.log("ERROR", "Failed to write response to: " + options.at("output"));
                    return 1;
                }
            }
            
            // Print response
            std::cout << "\nLLM Response:\n";
            std::cout << "=============\n\n";
            std::cout << response.content << std::endl;
            std::cout << "\nModel: " << response.model;
            std::cout << " | Tokens: " << response.totalTokens;
            std::cout << " | Time: " << response.processingTime << "ms" << std::endl;
            
            return 0;
            
        } catch (const std::exception& e) {
            logger.log("ERROR", "Exception during LLM execution: " + std::string(e.what()));
            return 1;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        FinancialDocumentAnalysisSystem system;
        system.showHelp();
        return 1;
    }

    std::string command = argv[1];
    FinancialDocumentAnalysisSystem system;

    if (command == "help" || command == "--help" || command == "-h") {
        system.showHelp();
        return 0;
    }
    else if (command == "build-index") {
        return system.buildIndex();
    }
    else if (command == "query") {
        if (argc < 3) {
            std::cout << "Error: Query text required" << std::endl;
            std::cout << "Usage: " << argv[0] << " query \"<query_text>\" [--top <number>] [--summary] [--json]" << std::endl;
            return 1;
        }

        std::string query = argv[2];
        int topK = 3;
        bool useSummary = false;
        bool useJSON = false;

        // Parse options
        for (int i = 3; i < argc; ++i) {
            std::string arg = argv[i];
            
            if (arg == "--top" && i + 1 < argc) {
                topK = std::stoi(argv[i + 1]);
                i++; // Skip next argument
            }
            else if (arg == "--summary") {
                useSummary = true;
            }
            else if (arg == "--json") {
                useJSON = true;
            }
        }

        return system.executeQuery(query, topK, useSummary, useJSON);
    }
    else if (command == "analyze") {
        return system.performAnalysis();
    }
    else if (command == "llm") {
        if (argc < 3) {
            std::cout << "Error: Prompt text required" << std::endl;
            std::cout << "Usage: " << argv[0] << " llm \"<prompt>\" [--file <path>] [--model <model>] [--max-tokens <number>] [--temperature <float>] [--output <path>]" << std::endl;
            return 1;
        }

        std::string prompt = argv[2];
        std::map<std::string, std::string> options;

        // Parse options
        for (int i = 3; i < argc; ++i) {
            std::string arg = argv[i];
            
            if (arg == "--file" && i + 1 < argc) {
                options["file"] = argv[++i];
            }
            else if (arg == "--model" && i + 1 < argc) {
                options["model"] = argv[++i];
            }
            else if (arg == "--max-tokens" && i + 1 < argc) {
                options["max-tokens"] = argv[++i];
            }
            else if (arg == "--temperature" && i + 1 < argc) {
                options["temperature"] = argv[++i];
            }
            else if (arg == "--output" && i + 1 < argc) {
                options["output"] = argv[++i];
            }
        }

        return system.executeLLM(prompt, options);
    }
    else {
        std::cout << "Error: Unknown command '" << command << "'" << std::endl;
        std::cout << "Use '" << argv[0] << " help' for usage information" << std::endl;
        return 1;
    }
}

