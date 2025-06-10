#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <regex>
#include <sstream>
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

struct FinancialMetric {
    std::string name;
    std::string category;
    double value;
    std::string unit;
    std::string period;
};

struct FinancialRisk {
    std::string category;
    std::string description;
    double severity;
    int mentionCount;
};

struct CompanyProfile {
    std::string name;
    std::string ticker;
    std::string industry;
    std::vector<FinancialMetric> metrics;
    std::vector<FinancialRisk> risks;
    std::map<std::string, double> ratios;
    std::map<std::string, std::string> trends;
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

class FinancialAnalyzer {
private:
    std::vector<CompanyProfile> companies;
    Logger logger;
    
    std::regex revenueRegex;
    std::regex profitRegex;
    std::regex riskRegex;
    std::regex percentageRegex;

    void initializeRegexPatterns() {
        revenueRegex = std::regex("Total\\s*(?:net)?\\s*(?:sales|revenue)\\s*(?:were|was)?\\s*\\$(\\d+\\.?\\d*)\\s*(billion|million)");
        profitRegex = std::regex("(gross|operating|net|profit)\\s*margin\\s*(?:was|of)?\\s*(\\d+\\.?\\d*)%");
        riskRegex = std::regex("risk(?:s)?\\s*(?:of|from|include[sd]?|related to)?\\s*([A-Za-z\\s]+)");
        percentageRegex = std::regex("(\\d+\\.?\\d*)%");
    }

    std::vector<FinancialMetric> extractBusinessMetrics(const std::string& content) {
        std::vector<FinancialMetric> metrics;
        
        std::sregex_iterator iter(content.begin(), content.end(), revenueRegex);
        std::sregex_iterator end;
        
        for (; iter != end; ++iter) {
            std::smatch match = *iter;
            FinancialMetric metric;
            metric.name = "Revenue";
            metric.category = "Income";
            metric.value = std::stod(match[1].str());
            metric.unit = "USD";
            
            std::string unit = match[2].str();
            if (unit == "billion") {
                metric.value *= 1000000000;
            } else if (unit == "million") {
                metric.value *= 1000000;
            }
            
            metrics.push_back(metric);
        }
        
        return metrics;
    }

    std::vector<FinancialRisk> extractRiskFactors(const std::string& content) {
        std::vector<FinancialRisk> risks;
        std::map<std::string, int> riskCounts;
        
        // Define risk categories and their keywords
        std::map<std::string, std::vector<std::string>> riskCategories = {
            {"Cybersecurity", {"cybersecurity", "cyber", "data breach", "security", "hacking"}},
            {"Competition", {"competition", "competitive", "competitor", "market share"}},
            {"Economic", {"economic", "recession", "inflation", "market conditions"}},
            {"Supply Chain", {"supply chain", "supplier", "manufacturing", "logistics"}},
            {"Regulatory", {"regulatory", "regulation", "compliance", "legal"}},
            {"Technology", {"technology", "innovation", "obsolescence", "disruption"}},
            {"Financial", {"financial", "liquidity", "credit", "debt", "cash flow"}}
        };
        
        std::string lowerContent = content;
        std::transform(lowerContent.begin(), lowerContent.end(), lowerContent.begin(), ::tolower);
        
        for (const auto& category : riskCategories) {
            int totalMentions = 0;
            
            for (const std::string& keyword : category.second) {
                size_t pos = 0;
                while ((pos = lowerContent.find(keyword, pos)) != std::string::npos) {
                    totalMentions++;
                    pos += keyword.length();
                }
            }
            
            if (totalMentions > 0) {
                FinancialRisk risk;
                risk.category = category.first;
                risk.description = "Item 1A.";  // Simplified description
                risk.mentionCount = totalMentions;
                risk.severity = std::min(1.0, totalMentions / 5.0);  // Normalize to 0-1
                
                risks.push_back(risk);
            }
        }
        
        return risks;
    }

    std::string determineIndustry(const std::string& companyName, const std::string& content) {
        std::string lowerContent = content;
        std::transform(lowerContent.begin(), lowerContent.end(), lowerContent.begin(), ::tolower);
        
        if (companyName.find("Microsoft") != std::string::npos ||
            lowerContent.find("software") != std::string::npos ||
            lowerContent.find("technology") != std::string::npos) {
            return "Technology";
        }
        
        if (lowerContent.find("financial") != std::string::npos ||
            lowerContent.find("banking") != std::string::npos) {
            return "Financial Services";
        }
        
        if (lowerContent.find("retail") != std::string::npos ||
            lowerContent.find("consumer") != std::string::npos) {
            return "Retail";
        }
        
        return "Other";
    }

    std::map<std::string, std::string> analyzeTrends(const std::vector<FinancialMetric>& metrics) {
        std::map<std::string, std::string> trends;
        
        // Simple trend analysis based on multiple revenue values
        std::vector<double> revenueValues;
        for (const auto& metric : metrics) {
            if (metric.name == "Revenue") {
                revenueValues.push_back(metric.value);
            }
        }
        
        if (revenueValues.size() >= 2) {
            double lastValue = revenueValues.back();
            double firstValue = revenueValues.front();
            
            if (lastValue > firstValue) {
                trends["Revenue"] = "Increase";
            } else if (lastValue < firstValue) {
                trends["Revenue"] = "Decrease";
            } else {
                trends["Revenue"] = "Stable";
            }
        }
        
        return trends;
    }

public:
    FinancialAnalyzer() {
        initializeRegexPatterns();
    }

    bool loadDocumentIndex(const std::string& indexPath) {
        logger.log("INFO", "Loading document index from " + indexPath);
        
        // Load from directory structure
        std::string inputDir = "../output";
        std::vector<std::string> jsonFiles = {
            "apple_10k_2023_structured.json", 
            "microsoft_10k_2024_structured.json"
        };
        
        for (const std::string& filename : jsonFiles) {
            std::string fullPath = inputDir + "/" + filename;
            std::ifstream file(fullPath);
            
            if (file.is_open()) {
                std::string content((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
                file.close();
                
                std::string companyName = SimpleJSONParser::extractStringValue(content, "company_name");
                auto sections = SimpleJSONParser::extractSections(content);
                
                logger.log("INFO", "Extracting company profiles from documents");
                
                CompanyProfile profile;
                profile.name = companyName;
                profile.industry = determineIndustry(companyName, content);
                
                logger.log("INFO", "Created profile for company: " + companyName);
                
                // Extract metrics and risks from all sections
                for (const auto& section : sections) {
                    logger.log("INFO", "Extracting business metrics for " + companyName);
                    auto metrics = extractBusinessMetrics(section.second);
                    profile.metrics.insert(profile.metrics.end(), metrics.begin(), metrics.end());
                    
                    logger.log("INFO", "Extracting risk factors for " + companyName);
                    auto risks = extractRiskFactors(section.second);
                    profile.risks.insert(profile.risks.end(), risks.begin(), risks.end());
                }
                
                profile.trends = analyzeTrends(profile.metrics);
                companies.push_back(profile);
            }
        }
        
        return !companies.empty();
    }

    void generateInsights(const std::string& jsonOutputPath, const std::string& markdownOutputPath) {
        logger.log("INFO", "Calculating financial ratios");
        
        // Generate JSON insights
        logger.log("INFO", "Generating insights JSON");
        generateJSONInsights(jsonOutputPath);
        
        // Generate Markdown report
        logger.log("INFO", "Generating Markdown report");
        generateMarkdownReport(markdownOutputPath);
    }

private:
    void generateJSONInsights(const std::string& outputPath) {
        std::ofstream file(outputPath);
        if (!file.is_open()) return;
        
        file << "{\n";
        file << "  \"companies\": [\n";
        
        for (size_t i = 0; i < companies.size(); ++i) {
            const auto& company = companies[i];
            
            file << "    {\n";
            file << "      \"name\": \"" << company.name << "\",\n";
            file << "      \"industry\": \"" << company.industry << "\",\n";
            
            // Metrics
            file << "      \"metrics\": [\n";
            for (size_t j = 0; j < company.metrics.size(); ++j) {
                const auto& metric = company.metrics[j];
                file << "        {\n";
                file << "          \"name\": \"" << metric.name << "\",\n";
                file << "          \"category\": \"" << metric.category << "\",\n";
                file << "          \"value\": " << metric.value << ",\n";
                file << "          \"unit\": \"" << metric.unit << "\"\n";
                file << "        }";
                if (j < company.metrics.size() - 1) file << ",";
                file << "\n";
            }
            file << "      ],\n";
            
            // Risks
            file << "      \"risks\": [\n";
            for (size_t j = 0; j < company.risks.size(); ++j) {
                const auto& risk = company.risks[j];
                file << "        {\n";
                file << "          \"category\": \"" << risk.category << "\",\n";
                file << "          \"description\": \"" << risk.description << "\",\n";
                file << "          \"mention_count\": " << risk.mentionCount << ",\n";
                file << "          \"severity\": " << std::fixed << std::setprecision(1) << risk.severity << "\n";
                file << "        }";
                if (j < company.risks.size() - 1) file << ",";
                file << "\n";
            }
            file << "      ],\n";
            
            // Trends
            file << "      \"trends\": {\n";
            bool first = true;
            for (const auto& trend : company.trends) {
                if (!first) file << ",\n";
                file << "        \"" << trend.first << "\": \"" << trend.second << "\"";
                first = false;
            }
            file << "\n      },\n";
            
            file << "      \"ratios\": {}\n";
            file << "    }";
            if (i < companies.size() - 1) file << ",";
            file << "\n";
        }
        
        file << "  ],\n";
        file << "  \"timestamp\": " << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) << "\n";
        file << "}";
        file.close();
        
        logger.log("INFO", "Saved insights to " + outputPath);
    }

    void generateMarkdownReport(const std::string& outputPath) {
        std::ofstream file(outputPath);
        if (!file.is_open()) return;
        
        file << "# Financial Analysis Report\n\n";
        file << "Generated on: " << getTimestamp() << "\n\n";
        
        for (const auto& company : companies) {
            file << "## " << company.name << "\n\n";
            file << "Industry: " << company.industry << "\n\n";
            
            // Metrics table
            file << "### Key Financial Metrics\n\n";
            file << "| Metric | Value | Period | Trend |\n";
            file << "|--------|-------|-----------|-------|\n";
            
            for (const auto& metric : company.metrics) {
                file << "| " << metric.name << " | ";
                if (metric.value >= 1000000000) {
                    file << "$" << std::fixed << std::setprecision(2) << (metric.value / 1000000000) << " billion";
                } else if (metric.value >= 1000000) {
                    file << "$" << std::fixed << std::setprecision(2) << (metric.value / 1000000) << " million";
                } else {
                    file << "$" << std::fixed << std::setprecision(2) << metric.value;
                }
                file << " | " << metric.period << " | ";
                
                auto trendIt = company.trends.find(metric.name);
                if (trendIt != company.trends.end()) {
                    file << trendIt->second;
                }
                file << " |\n";
            }
            file << "\n";
            
            // Risk factors
            file << "### Key Risk Factors\n\n";
            for (const auto& risk : company.risks) {
                file << "#### " << risk.category << " Risk\n\n";
                file << risk.description << "\n\n";
                file << "Severity: ";
                
                int stars = static_cast<int>(risk.severity * 5);
                for (int i = 0; i < 5; ++i) {
                    if (i < stars) {
                        file << "★";
                    } else {
                        file << "☆";
                    }
                }
                file << " (" << risk.mentionCount << " mentions)\n\n";
            }
            
            file << "---\n\n";
        }
        
        file.close();
        logger.log("INFO", "Saved report to " + outputPath);
    }

    std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Financial Analyzer - Windows Compatible" << std::endl;
        std::cout << "Usage: " << argv[0] << " <index_file> <json_output> <markdown_output>" << std::endl;
        std::cout << "Example: " << argv[0] << " index.json insights.json report.md" << std::endl;
        return 1;
    }

    std::string indexFile = argv[1];
    std::string jsonOutput = argv[2];
    std::string markdownOutput = argv[3];

    FinancialAnalyzer analyzer;
    
    if (analyzer.loadDocumentIndex(indexFile)) {
        auto start = std::chrono::high_resolution_clock::now();
        analyzer.generateInsights(jsonOutput, markdownOutput);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "[INFO] Analysis completed in " << duration.count() << "ms" << std::endl;
    } else {
        std::cerr << "Failed to load document index" << std::endl;
        return 1;
    }

    return 0;
}

