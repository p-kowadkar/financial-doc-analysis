#ifndef SEC_EDGAR_CLIENT_H
#define SEC_EDGAR_CLIENT_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <thread>

struct CompanyInfo {
    std::string cik;
    std::string name;
    std::string ticker;
    std::string sic;
    std::string sicDescription;
    std::string stateOfIncorporation;
    std::string fiscalYearEnd;
};

struct Filing {
    std::string accessionNumber;
    std::string filingDate;
    std::string reportDate;
    std::string acceptanceDateTime;
    std::string act;
    std::string form;
    std::string fileNumber;
    std::string filmNumber;
    std::string items;
    int size;
    bool isXBRL;
    bool isInlineXBRL;
    std::string primaryDocument;
    std::string primaryDocDescription;
};

struct SECConfig {
    std::string userAgent;
    std::string outputDirectory;
    std::vector<std::string> targetCIKs;
    std::vector<std::string> filingTypes;
    std::vector<int> targetYears;
    int maxFilingsPerCompany;
    int requestDelayMs;
    bool downloadDocuments;
    bool extractXBRL;
};

class SECEdgarClient {
private:
    SECConfig config;
    std::chrono::steady_clock::time_point lastRequestTime;
    
    // Rate limiting
    void enforceRateLimit();
    
    // HTTP utilities
    std::string makeHttpRequest(const std::string& url);
    bool downloadFile(const std::string& url, const std::string& filepath);
    
    // JSON parsing utilities
    std::string extractJsonString(const std::string& json, const std::string& key);
    std::vector<std::string> extractJsonArray(const std::string& json, const std::string& key);
    
    // CIK utilities
    std::string formatCIK(const std::string& cik);
    std::string lookupCIKByTicker(const std::string& ticker);
    
    // File utilities
    std::string sanitizeFilename(const std::string& filename);
    bool createDirectoryIfNotExists(const std::string& path);

public:
    explicit SECEdgarClient(const SECConfig& config);
    ~SECEdgarClient();
    
    // Configuration
    bool loadConfigFromFile(const std::string& configPath);
    void saveConfigToFile(const std::string& configPath) const;
    
    // Company information
    CompanyInfo getCompanyInfo(const std::string& cik);
    std::vector<std::string> searchCompaniesByName(const std::string& name);
    
    // Filing retrieval
    std::vector<Filing> getCompanyFilings(const std::string& cik, 
                                         const std::string& filingType = "",
                                         int maxResults = 100);
    
    // Document download
    bool downloadFiling(const Filing& filing, const std::string& outputPath = "");
    bool downloadCompanyFilings(const std::string& cik, 
                               const std::vector<std::string>& filingTypes,
                               int maxPerType = 10);
    
    // Bulk operations
    bool downloadBulkSubmissions();
    bool downloadBulkCompanyFacts();
    
    // XBRL data
    std::string getCompanyFacts(const std::string& cik);
    std::string getCompanyConcept(const std::string& cik, 
                                 const std::string& taxonomy,
                                 const std::string& tag);
    
    // Utility functions
    std::vector<std::string> getAvailableFilingTypes();
    bool validateCIK(const std::string& cik);
    std::string getLastError() const;
    
private:
    std::string lastError;
};

#endif // SEC_EDGAR_CLIENT_H

