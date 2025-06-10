# Financial Document Analysis System

A C++ system for analyzing financial documents using natural language processing and machine learning techniques. The system processes SEC filings (10-K, 10-Q) and provides semantic search, risk analysis, and financial metrics extraction capabilities.

## Features

- **Document Processing**
  - HTML text extraction
  - Document parsing and structuring
  - TF-IDF based semantic search
  - Extractive summarization

- **Financial Analysis**
  - Risk factor identification and scoring
  - Financial metrics extraction
  - Company performance analysis
  - Cross-company comparisons

- **SEC Integration**
  - EDGAR API integration
  - Company information retrieval
  - Automated filing downloads
  - Bulk processing support

- **LLM Integration**
  - OpenRouter API support
  - Multiple model support with fallback
  - Response caching
  - Rate limiting

## Requirements

- C++17 compatible compiler (g++, MSVC, or clang++)
- libcurl for API connectivity
- vcpkg package manager
- Windows 10/11 or Unix-like OS

## Installation

1. Clone the repository:
```bash
git clone https://github.com/yourusername/financial-doc-analysis.git
cd financial-doc-analysis
```

2. Install dependencies using vcpkg:
```bash
vcpkg install curl:x64-windows
```

3. Build the project:
```bash
# Windows
cd src
build.bat

# Unix
cd src
./build.sh
```

## Usage

### Document Analysis
```bash
# Build document index
main.exe build-index

# Search documents
main.exe query "cybersecurity risks" --top 3 --summary

# Analyze documents
main.exe analyze
```

### SEC Data Acquisition
```bash
# Get company information
sec_data_acquisition.exe company-info 0000320193

# Download filings
sec_data_acquisition.exe download-company 0000320193 10-K 10-Q
```

### LLM Integration
```bash
# Process with LLM
main.exe llm "Analyze revenue trends" --file ../data/apple_10k_2023_sample.txt

# Custom model selection
main.exe llm "Compare growth metrics" --model deepseek-chat
```

## Configuration

1. Create `config/llm_config.conf`:
```json
{
  "api_key": "your-openrouter-api-key",
  "base_url": "https://openrouter.ai/api/v1",
  "preferred_models": [
    "deepseek-chat",
    "gpt-3.5-turbo"
  ]
}
```

2. Set environment variables (optional):
```bash
export OPENROUTER_API_KEY=your-api-key
```

## Project Structure

```
financial-doc-analysis/
├── config/               # Configuration files
├── data/                # Sample documents
├── docs/                # Documentation
├── output/              # Analysis outputs
├── src/                 # Source code
│   ├── document_parser/
│   ├── financial_analyzer/
│   ├── llm_client/
│   └── query_engine/
├── tests/               # Test suite
│   ├── unit/
│   └── integration/
└── web_demo/            # Web interface
```

## Testing

Run the test suite:
```bash
cd tests
./run_tests.sh
```

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- OpenRouter API for LLM capabilities
- SEC EDGAR system for financial data
- Contributors and maintainers
