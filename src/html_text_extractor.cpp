#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <algorithm>

class HTMLTextExtractor {
private:
    std::string removeHTMLTags(const std::string& html) {
        std::string text = html;
        
        // Remove HTML tags
        std::regex tagRegex("<[^>]*>");
        text = std::regex_replace(text, tagRegex, " ");
        
        // Decode common HTML entities
        std::regex ampRegex("&amp;");
        text = std::regex_replace(text, ampRegex, "&");
        
        std::regex ltRegex("&lt;");
        text = std::regex_replace(text, ltRegex, "<");
        
        std::regex gtRegex("&gt;");
        text = std::regex_replace(text, gtRegex, ">");
        
        std::regex quotRegex("&quot;");
        text = std::regex_replace(text, quotRegex, "\"");
        
        std::regex nbspRegex("&nbsp;");
        text = std::regex_replace(text, nbspRegex, " ");
        
        // Normalize whitespace
        std::regex whitespaceRegex("\\s+");
        text = std::regex_replace(text, whitespaceRegex, " ");
        
        // Trim leading and trailing whitespace
        text.erase(text.begin(), std::find_if(text.begin(), text.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        text.erase(std::find_if(text.rbegin(), text.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), text.end());
        
        return text;
    }

public:
    bool extractText(const std::string& inputFile, const std::string& outputFile) {
        std::ifstream file(inputFile);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open input file: " << inputFile << std::endl;
            return false;
        }

        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();

        std::string cleanText = removeHTMLTags(content);

        std::ofstream outFile(outputFile);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not create output file: " << outputFile << std::endl;
            return false;
        }

        outFile << cleanText;
        outFile.close();

        std::cout << "Successfully extracted text from " << inputFile << " to " << outputFile << std::endl;
        std::cout << "Original size: " << content.length() << " characters" << std::endl;
        std::cout << "Cleaned size: " << cleanText.length() << " characters" << std::endl;

        return true;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "HTML Text Extractor - Windows Compatible" << std::endl;
        std::cout << "Usage: " << argv[0] << " <input_file> <output_file>" << std::endl;
        std::cout << "Example: " << argv[0] << " document.html document.txt" << std::endl;
        return 1;
    }

    HTMLTextExtractor extractor;
    if (extractor.extractText(argv[1], argv[2])) {
        return 0;
    } else {
        return 1;
    }
}

