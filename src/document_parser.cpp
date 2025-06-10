#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <sstream>
#include <algorithm>

class SimpleJSONBuilder {
private:
    std::string escapeString(const std::string& str) {
        std::string escaped = str;
        
        // Replace backslashes first
        size_t pos = 0;
        while ((pos = escaped.find("\\", pos)) != std::string::npos) {
            escaped.replace(pos, 1, "\\\\");
            pos += 2;
        }
        
        // Replace quotes
        pos = 0;
        while ((pos = escaped.find("\"", pos)) != std::string::npos) {
            escaped.replace(pos, 1, "\\\"");
            pos += 2;
        }
        
        // Replace newlines
        pos = 0;
        while ((pos = escaped.find("\n", pos)) != std::string::npos) {
            escaped.replace(pos, 1, "\\n");
            pos += 2;
        }
        
        // Replace carriage returns
        pos = 0;
        while ((pos = escaped.find("\r", pos)) != std::string::npos) {
            escaped.replace(pos, 1, "\\r");
            pos += 2;
        }
        
        return escaped;
    }

public:
    std::string createDocumentJSON(const std::string& companyName, 
                                  const std::string& fiscalYear,
                                  const std::vector<std::pair<std::string, std::string>>& sections) {
        std::stringstream json;
        
        json << "{\n";
        json << "  \"metadata\": {\n";
        json << "    \"company_name\": \"" << escapeString(companyName) << "\",\n";
        json << "    \"fiscal_year_end\": \"" << escapeString(fiscalYear) << "\"\n";
        json << "  },\n";
        json << "  \"sections\": [\n";
        
        for (size_t i = 0; i < sections.size(); ++i) {
            json << "    {\n";
            json << "      \"title\": \"" << escapeString(sections[i].first) << "\",\n";
            json << "      \"content\": \"" << escapeString(sections[i].second) << "\"\n";
            json << "    }";
            
            if (i < sections.size() - 1) {
                json << ",";
            }
            json << "\n";
        }
        
        json << "  ]\n";
        json << "}";
        
        return json.str();
    }
};

class DocumentParser {
private:
    std::string extractSimpleSection(const std::string& content, const std::string& keyword, int maxLength = 1500) {
        // Convert to lowercase for case-insensitive search
        std::string lowerContent = content;
        std::string lowerKeyword = keyword;
        std::transform(lowerContent.begin(), lowerContent.end(), lowerContent.begin(), ::tolower);
        std::transform(lowerKeyword.begin(), lowerKeyword.end(), lowerKeyword.begin(), ::tolower);
        
        size_t pos = lowerContent.find(lowerKeyword);
        if (pos != std::string::npos) {
            size_t start = pos;
            size_t end = std::min(content.length(), start + maxLength);
            return content.substr(start, end - start);
        }
        return "";
    }

    std::string extractCompanyName(const std::string& content) {
        // Look for common company patterns
        std::vector<std::string> patterns = {
            "Apple Inc.",
            "Microsoft Corporation",
            "Amazon.com, Inc.",
            "Alphabet Inc.",
            "Meta Platforms, Inc."
        };
        
        for (const std::string& pattern : patterns) {
            if (content.find(pattern) != std::string::npos) {
                return pattern;
            }
        }
        
        // Generic extraction
        std::regex companyRegex("([A-Z][a-zA-Z\\s]+(?:Inc\\.|Corporation|Corp\\.|LLC|Ltd\\.))");
        std::smatch match;
        if (std::regex_search(content, match, companyRegex)) {
            return match[1].str();
        }
        
        return "Unknown Company";
    }

    std::string extractFiscalYear(const std::string& content) {
        std::regex fiscalRegex("fiscal\\s*year\\s*end(?:ed|ing)?\\s*([A-Za-z]+\\s*\\d+,?\\s*\\d{4})");
        std::smatch match;
        if (std::regex_search(content, match, fiscalRegex)) {
            return match[1].str();
        }
        
        // Look for year patterns
        std::regex yearRegex("(\\d{4})");
        if (std::regex_search(content, match, yearRegex)) {
            return match[1].str();
        }
        
        return "Unknown";
    }

public:
    bool parseDocument(const std::string& inputFile, const std::string& outputFile) {
        std::ifstream file(inputFile);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open input file: " << inputFile << std::endl;
            return false;
        }

        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();

        // Extract metadata
        std::string companyName = extractCompanyName(content);
        std::string fiscalYear = extractFiscalYear(content);

        // Extract sections using keyword search
        std::vector<std::pair<std::string, std::string>> sections;
        
        std::string businessSection = extractSimpleSection(content, "Business", 2000);
        if (!businessSection.empty()) {
            sections.push_back({"business", businessSection});
        }

        std::string riskSection = extractSimpleSection(content, "Risk Factors", 2500);
        if (!riskSection.empty()) {
            sections.push_back({"risk_factors", riskSection});
        }

        std::string propertiesSection = extractSimpleSection(content, "Properties", 1000);
        if (!propertiesSection.empty()) {
            sections.push_back({"properties", propertiesSection});
        }

        std::string legalSection = extractSimpleSection(content, "Legal Proceedings", 1000);
        if (!legalSection.empty()) {
            sections.push_back({"legal_proceedings", legalSection});
        }

        std::string marketSection = extractSimpleSection(content, "Market", 1500);
        if (!marketSection.empty()) {
            sections.push_back({"market_information", marketSection});
        }

        std::string mdaSection = extractSimpleSection(content, "Management", 2000);
        if (!mdaSection.empty()) {
            sections.push_back({"management_discussion", mdaSection});
        }

        std::string financialSection = extractSimpleSection(content, "Financial Statements", 2000);
        if (!financialSection.empty()) {
            sections.push_back({"financial_statements", financialSection});
        }

        // Generate JSON using simple string builder
        SimpleJSONBuilder jsonBuilder;
        std::string jsonOutput = jsonBuilder.createDocumentJSON(companyName, fiscalYear, sections);

        // Write output
        std::ofstream outFile(outputFile);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not create output file: " << outputFile << std::endl;
            return false;
        }

        outFile << jsonOutput;
        outFile.close();

        std::cout << "Successfully parsed document and saved to: " << outputFile << std::endl;
        std::cout << "Company: " << companyName << std::endl;
        std::cout << "Fiscal Year: " << fiscalYear << std::endl;
        std::cout << "Sections extracted: " << sections.size() << std::endl;

        return true;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Document Parser - Windows Compatible" << std::endl;
        std::cout << "Usage: " << argv[0] << " <input_file> <output_file>" << std::endl;
        std::cout << "Example: " << argv[0] << " document.txt output.json" << std::endl;
        return 1;
    }

    DocumentParser parser;
    if (parser.parseDocument(argv[1], argv[2])) {
        return 0;
    } else {
        return 1;
    }
}

