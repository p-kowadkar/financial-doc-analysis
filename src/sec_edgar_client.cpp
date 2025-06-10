#include "sec_edgar_client.h"
#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <filesystem>
#include <iomanip>
#include <algorithm>

// Callback function for libcurl to write data to string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Callback function for libcurl to write data to file
static size_t WriteFileCallback(void* contents, size_t size, size_t nmemb, FILE* file) {
    return fwrite(contents, size, nmemb, file);
}

SECEdgarClient::SECEdgarClient(const SECConfig& config) : config(config) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    lastRequestTime = std::chrono::steady_clock::now();
}

SECEdgarClient::~SECEdgarClient() {
    curl_global_cleanup();
}

void SECEdgarClient::enforceRateLimit() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRequestTime);
    
    if (elapsed.count() < config.requestDelayMs) {
        std::this_thread::sleep_for(std::chrono::milliseconds(config.requestDelayMs - elapsed.count()));
    }
    
    lastRequestTime = std::chrono::steady_clock::now();
}

std::string SECEdgarClient::makeHttpRequest(const std::string& url) {
    enforceRateLimit();
    
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
    
    // Set User-Agent (required by SEC)
    curl_easy_setopt(curl, CURLOPT_USERAGENT, config.userAgent.c_str());
    
    // Set callback function
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    
    // Follow redirects
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    // Set timeout
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    
    // Perform request
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        lastError = "CURL request failed: " + std::string(curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return "";
    }
    
    // Check HTTP response code
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    
    curl_easy_cleanup(curl);
    
    if (response_code != 200) {
        lastError = "HTTP error: " + std::to_string(response_code);
        return "";
    }
    
    return readBuffer;
}

bool SECEdgarClient::downloadFile(const std::string& url, const std::string& filepath) {
    enforceRateLimit();
    
    CURL* curl;
    CURLcode res;
    FILE* file;
    
    // Create directory if it doesn't exist
    std::filesystem::path path(filepath);
    std::filesystem::create_directories(path.parent_path());
    
    file = fopen(filepath.c_str(), "wb");
    if (!file) {
        lastError = "Cannot open file for writing: " + filepath;
        return false;
    }
    
    curl = curl_easy_init();
    if (!curl) {
        fclose(file);
        lastError = "Failed to initialize CURL";
        return false;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, config.userAgent.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
    
    res = curl_easy_perform(curl);
    
    fclose(file);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        lastError = "Download failed: " + std::string(curl_easy_strerror(res));
        std::filesystem::remove(filepath);
        return false;
    }
    
    return true;
}

std::string SECEdgarClient::formatCIK(const std::string& cik) {
    // Remove any non-digit characters
    std::string cleanCIK;
    for (char c : cik) {
        if (std::isdigit(c)) {
            cleanCIK += c;
        }
    }
    
    // Pad with leading zeros to make it 10 digits
    while (cleanCIK.length() < 10) {
        cleanCIK = "0" + cleanCIK;
    }
    
    return cleanCIK;
}

std::string SECEdgarClient::extractJsonString(const std::string& json, const std::string& key) {
    // Simple JSON string extraction (for production, consider using a proper JSON library)
    std::regex pattern("\"" + key + "\"\\s*:\\s*\"([^\"]+)\"");
    std::smatch match;
    
    if (std::regex_search(json, match, pattern)) {
        return match[1].str();
    }
    
    return "";
}

std::vector<std::string> SECEdgarClient::extractJsonArray(const std::string& json, const std::string& key) {
    std::vector<std::string> result;
    
    // Find the array start
    std::string searchPattern = "\"" + key + "\"\\s*:\\s*\\[";
    std::regex arrayStart(searchPattern);
    std::smatch match;
    
    if (!std::regex_search(json, match, arrayStart)) {
        return result;
    }
    
    // Find array content (simplified parsing)
    size_t startPos = match.position() + match.length();
    size_t endPos = json.find(']', startPos);
    
    if (endPos == std::string::npos) {
        return result;
    }
    
    std::string arrayContent = json.substr(startPos, endPos - startPos);
    
    // Extract quoted strings from array
    std::regex stringPattern("\"([^\"]+)\"");
    std::sregex_iterator iter(arrayContent.begin(), arrayContent.end(), stringPattern);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        result.push_back((*iter)[1].str());
    }
    
    return result;
}

CompanyInfo SECEdgarClient::getCompanyInfo(const std::string& cik) {
    CompanyInfo info;
    std::string formattedCIK = formatCIK(cik);
    std::string url = "https://data.sec.gov/submissions/CIK" + formattedCIK + ".json";
    
    std::string response = makeHttpRequest(url);
    if (response.empty()) {
        return info;
    }
    
    info.cik = formattedCIK;
    info.name = extractJsonString(response, "name");
    info.sic = extractJsonString(response, "sic");
    info.sicDescription = extractJsonString(response, "sicDescription");
    info.stateOfIncorporation = extractJsonString(response, "stateOfIncorporation");
    info.fiscalYearEnd = extractJsonString(response, "fiscalYearEnd");
    
    // Extract ticker symbols
    std::vector<std::string> tickers = extractJsonArray(response, "tickers");
    if (!tickers.empty()) {
        info.ticker = tickers[0]; // Use the first ticker
    }
    
    return info;
}

std::vector<Filing> SECEdgarClient::getCompanyFilings(const std::string& cik, 
                                                     const std::string& filingType,
                                                     int maxResults) {
    std::vector<Filing> filings;
    std::string formattedCIK = formatCIK(cik);
    std::string url = "https://data.sec.gov/submissions/CIK" + formattedCIK + ".json";
    
    std::string response = makeHttpRequest(url);
    if (response.empty()) {
        return filings;
    }
    
    // Extract filing arrays (simplified parsing)
    std::vector<std::string> accessionNumbers = extractJsonArray(response, "accessionNumber");
    std::vector<std::string> filingDates = extractJsonArray(response, "filingDate");
    std::vector<std::string> reportDates = extractJsonArray(response, "reportDate");
    std::vector<std::string> forms = extractJsonArray(response, "form");
    std::vector<std::string> primaryDocuments = extractJsonArray(response, "primaryDocument");
    
    // Build filing objects
    size_t count = std::min({accessionNumbers.size(), filingDates.size(), 
                            reportDates.size(), forms.size()});
    
    for (size_t i = 0; i < count && filings.size() < maxResults; ++i) {
        if (!filingType.empty() && forms[i] != filingType) {
            continue;
        }
        
        Filing filing;
        filing.accessionNumber = accessionNumbers[i];
        filing.filingDate = filingDates[i];
        filing.reportDate = reportDates[i];
        filing.form = forms[i];
        
        if (i < primaryDocuments.size()) {
            filing.primaryDocument = primaryDocuments[i];
        }
        
        filings.push_back(filing);
    }
    
    return filings;
}

bool SECEdgarClient::downloadFiling(const Filing& filing, const std::string& outputPath) {
    if (filing.accessionNumber.empty() || filing.primaryDocument.empty()) {
        lastError = "Invalid filing information";
        return false;
    }
    
    // Construct EDGAR URL
    std::string accessionNoSlashes = filing.accessionNumber;
    accessionNoSlashes.erase(std::remove(accessionNoSlashes.begin(), 
                                        accessionNoSlashes.end(), '-'), 
                            accessionNoSlashes.end());
    
    std::string url = "https://www.sec.gov/Archives/edgar/data/" + 
                     std::to_string(std::stoi(filing.accessionNumber.substr(0, 10))) + "/" +
                     accessionNoSlashes + "/" + filing.primaryDocument;
    
    std::string filepath = outputPath.empty() ? 
                          config.outputDirectory + "/" + filing.form + "_" + 
                          filing.accessionNumber + "_" + filing.primaryDocument :
                          outputPath;
    
    return downloadFile(url, filepath);
}

bool SECEdgarClient::downloadCompanyFilings(const std::string& cik, 
                                           const std::vector<std::string>& filingTypes,
                                           int maxPerType) {
    std::string formattedCIK = formatCIK(cik);
    CompanyInfo company = getCompanyInfo(formattedCIK);
    
    if (company.name.empty()) {
        lastError = "Could not retrieve company information for CIK: " + formattedCIK;
        return false;
    }
    
    std::cout << "Downloading filings for: " << company.name << " (CIK: " << formattedCIK << ")" << std::endl;
    
    bool success = true;
    for (const std::string& filingType : filingTypes) {
        std::cout << "Fetching " << filingType << " filings..." << std::endl;
        
        std::vector<Filing> filings = getCompanyFilings(formattedCIK, filingType, maxPerType);
        
        std::cout << "Found " << filings.size() << " " << filingType << " filings" << std::endl;
        
        for (const Filing& filing : filings) {
            std::string outputDir = config.outputDirectory + "/" + 
                                   sanitizeFilename(company.name) + "/" + filingType;
            
            if (!createDirectoryIfNotExists(outputDir)) {
                std::cerr << "Failed to create directory: " << outputDir << std::endl;
                success = false;
                continue;
            }
            
            std::string filepath = outputDir + "/" + filing.accessionNumber + "_" + 
                                  filing.primaryDocument;
            
            std::cout << "Downloading: " << filing.accessionNumber << " (" << 
                        filing.filingDate << ")" << std::endl;
            
            if (!downloadFiling(filing, filepath)) {
                std::cerr << "Failed to download filing: " << filing.accessionNumber << 
                            " - " << lastError << std::endl;
                success = false;
            }
        }
    }
    
    return success;
}

std::string SECEdgarClient::sanitizeFilename(const std::string& filename) {
    std::string result = filename;
    
    // Replace invalid characters with underscores
    std::regex invalidChars("[<>:\"/\\\\|?*]");
    result = std::regex_replace(result, invalidChars, "_");
    
    // Remove multiple consecutive underscores
    std::regex multipleUnderscores("_{2,}");
    result = std::regex_replace(result, multipleUnderscores, "_");
    
    // Trim underscores from start and end
    result.erase(0, result.find_first_not_of('_'));
    result.erase(result.find_last_not_of('_') + 1);
    
    return result;
}

bool SECEdgarClient::createDirectoryIfNotExists(const std::string& path) {
    try {
        return std::filesystem::create_directories(path);
    } catch (const std::exception& e) {
        lastError = "Failed to create directory: " + std::string(e.what());
        return false;
    }
}

bool SECEdgarClient::loadConfigFromFile(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        lastError = "Cannot open config file: " + configPath;
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        // Trim whitespace
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        if (key == "user_agent") {
            config.userAgent = value;
        } else if (key == "output_directory") {
            config.outputDirectory = value;
        } else if (key == "max_filings_per_company") {
            config.maxFilingsPerCompany = std::stoi(value);
        } else if (key == "request_delay_ms") {
            config.requestDelayMs = std::stoi(value);
        } else if (key == "download_documents") {
            config.downloadDocuments = (value == "true" || value == "1");
        }
    }
    
    return true;
}

std::string SECEdgarClient::getLastError() const {
    return lastError;
}

bool SECEdgarClient::validateCIK(const std::string& cik) {
    std::string formattedCIK = formatCIK(cik);
    return formattedCIK.length() == 10 && std::all_of(formattedCIK.begin(), formattedCIK.end(), ::isdigit);
}

