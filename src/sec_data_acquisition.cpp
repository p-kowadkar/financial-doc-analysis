#include "sec_edgar_client.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>

void printUsage(const char* programName) {
    std::cout << "SEC EDGAR Data Acquisition Tool" << std::endl;
    std::cout << "Usage: " << programName << " <command> [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  download-company <CIK> [filing_types...]  Download filings for a specific company" << std::endl;
    std::cout << "  company-info <CIK>                        Get company information" << std::endl;
    std::cout << "  list-filings <CIK> [filing_type]          List available filings" << std::endl;
    std::cout << "  bulk-download <config_file>               Download multiple companies from config" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << " download-company 0000320193 10-K 10-Q" << std::endl;
    std::cout << "  " << programName << " company-info 0000789019" << std::endl;
    std::cout << "  " << programName << " list-filings 0000320193 10-K" << std::endl;
    std::cout << "  " << programName << " bulk-download companies.conf" << std::endl;
    std::cout << std::endl;
    std::cout << "Note: CIK should be 10-digit format with leading zeros" << std::endl;
    std::cout << "      Common CIKs: Apple (0000320193), Microsoft (0000789019)" << std::endl;
}

void printCompanyInfo(const CompanyInfo& info) {
    std::cout << "Company Information:" << std::endl;
    std::cout << "  Name: " << info.name << std::endl;
    std::cout << "  CIK: " << info.cik << std::endl;
    std::cout << "  Ticker: " << info.ticker << std::endl;
    std::cout << "  SIC: " << info.sic << " (" << info.sicDescription << ")" << std::endl;
    std::cout << "  State: " << info.stateOfIncorporation << std::endl;
    std::cout << "  Fiscal Year End: " << info.fiscalYearEnd << std::endl;
}

void printFilings(const std::vector<Filing>& filings) {
    std::cout << "Available Filings:" << std::endl;
    std::cout << "  Form      Filing Date   Report Date   Accession Number" << std::endl;
    std::cout << "  --------  ------------  ------------  --------------------" << std::endl;
    
    for (const Filing& filing : filings) {
        std::cout << "  " << std::left << std::setw(8) << filing.form
                  << "  " << std::setw(12) << filing.filingDate
                  << "  " << std::setw(12) << filing.reportDate
                  << "  " << filing.accessionNumber << std::endl;
    }
}

SECConfig createDefaultConfig() {
    SECConfig config;
    config.userAgent = "Financial Analysis System 1.0 (contact@example.com)";
    config.outputDirectory = "../data/sec_filings";
    config.maxFilingsPerCompany = 10;
    config.requestDelayMs = 100; // 10 requests per second
    config.downloadDocuments = true;
    config.extractXBRL = false;
    
    return config;
}

bool loadCompaniesFromConfig(const std::string& configFile, std::vector<std::string>& ciks, 
                            std::vector<std::string>& filingTypes) {
    std::ifstream file(configFile);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open config file: " << configFile << std::endl;
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        if (line.find("CIK=") == 0) {
            ciks.push_back(line.substr(4));
        } else if (line.find("FILING_TYPE=") == 0) {
            filingTypes.push_back(line.substr(12));
        }
    }
    
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string command = argv[1];
    SECConfig config = createDefaultConfig();
    SECEdgarClient client(config);
    
    if (command == "company-info") {
        if (argc < 3) {
            std::cerr << "Error: CIK required for company-info command" << std::endl;
            return 1;
        }
        
        std::string cik = argv[2];
        std::cout << "Fetching company information for CIK: " << cik << std::endl;
        
        CompanyInfo info = client.getCompanyInfo(cik);
        if (info.name.empty()) {
            std::cerr << "Error: " << client.getLastError() << std::endl;
            return 1;
        }
        
        printCompanyInfo(info);
        
    } else if (command == "list-filings") {
        if (argc < 3) {
            std::cerr << "Error: CIK required for list-filings command" << std::endl;
            return 1;
        }
        
        std::string cik = argv[2];
        std::string filingType = (argc > 3) ? argv[3] : "";
        
        std::cout << "Fetching filings for CIK: " << cik;
        if (!filingType.empty()) {
            std::cout << " (Type: " << filingType << ")";
        }
        std::cout << std::endl;
        
        std::vector<Filing> filings = client.getCompanyFilings(cik, filingType, 20);
        if (filings.empty()) {
            std::cerr << "Error: " << client.getLastError() << std::endl;
            return 1;
        }
        
        printFilings(filings);
        
    } else if (command == "download-company") {
        if (argc < 3) {
            std::cerr << "Error: CIK required for download-company command" << std::endl;
            return 1;
        }
        
        std::string cik = argv[2];
        std::vector<std::string> filingTypes;
        
        // Default to 10-K and 10-Q if no types specified
        if (argc == 3) {
            filingTypes = {"10-K", "10-Q"};
        } else {
            for (int i = 3; i < argc; ++i) {
                filingTypes.push_back(argv[i]);
            }
        }
        
        std::cout << "Downloading filings for CIK: " << cik << std::endl;
        std::cout << "Filing types: ";
        for (const std::string& type : filingTypes) {
            std::cout << type << " ";
        }
        std::cout << std::endl;
        
        if (!client.downloadCompanyFilings(cik, filingTypes, config.maxFilingsPerCompany)) {
            std::cerr << "Error: " << client.getLastError() << std::endl;
            return 1;
        }
        
        std::cout << "Download completed successfully!" << std::endl;
        
    } else if (command == "bulk-download") {
        if (argc < 3) {
            std::cerr << "Error: Config file required for bulk-download command" << std::endl;
            return 1;
        }
        
        std::string configFile = argv[2];
        std::vector<std::string> ciks;
        std::vector<std::string> filingTypes;
        
        if (!loadCompaniesFromConfig(configFile, ciks, filingTypes)) {
            return 1;
        }
        
        if (ciks.empty()) {
            std::cerr << "Error: No CIKs found in config file" << std::endl;
            return 1;
        }
        
        if (filingTypes.empty()) {
            filingTypes = {"10-K", "10-Q"}; // Default types
        }
        
        std::cout << "Starting bulk download for " << ciks.size() << " companies" << std::endl;
        
        for (const std::string& cik : ciks) {
            std::cout << "\n--- Processing CIK: " << cik << " ---" << std::endl;
            
            if (!client.downloadCompanyFilings(cik, filingTypes, config.maxFilingsPerCompany)) {
                std::cerr << "Warning: Failed to download filings for CIK " << cik 
                          << " - " << client.getLastError() << std::endl;
                continue;
            }
        }
        
        std::cout << "\nBulk download completed!" << std::endl;
        
    } else {
        std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    return 0;
}

