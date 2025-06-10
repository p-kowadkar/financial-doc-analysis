# Financial Document Analysis System

[![CI/CD Pipeline](https://github.com/p-kowadkar/financial-doc-analysis/actions/workflows/ci.yml/badge.svg)](https://github.com/p-kowadkar/financial-doc-analysis/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A comprehensive C++ system for analyzing financial documents using advanced text processing, TF-IDF embeddings, and LLM integration with SEC EDGAR data acquisition capabilities.

## 🚀 Features

### Core Capabilities
- **📄 Document Processing**: Parse and extract text from various financial document formats
- **🔍 TF-IDF Embeddings**: Generate semantic embeddings for document similarity and search
- **🤖 LLM Integration**: Connect with language models for advanced analysis and querying
- **🧠 RAG Engine**: Retrieval-Augmented Generation for contextual financial insights
- **❓ Query System**: Natural language querying of financial document collections
- **🌐 Web Interface**: Interactive web demo for document analysis
- **📊 SEC Data Acquisition**: Direct integration with SEC EDGAR database

### New Features
- **🔄 CI/CD Pipeline**: Automated testing and deployment
- **🌿 Branch Protection**: Professional development workflow
- **📋 Comprehensive Testing**: Unit, integration, and system tests
- **📚 Documentation**: Complete setup and development guides

## 🏗️ Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   SEC EDGAR     │    │   Document      │    │   TF-IDF        │
│   Client        │───▶│   Parser        │───▶│   Embedding     │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                                                        │
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Web Demo      │    │   Query Engine  │    │   RAG Engine    │
│   Interface     │◀───│   & LLM Client  │◀───│   System        │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## 🚀 Quick Start

### Prerequisites
- **Compiler**: C++17 compatible (g++, MSVC, MinGW)
- **Platform**: Windows/Linux/macOS
- **Optional**: Python 3.x for web demo
- **Git**: For version control and development

### Installation

#### 1. Clone Repository
```bash
git clone https://github.com/p-kowadkar/financial-doc-analysis.git
cd financial-doc-analysis
```

#### 2. Build System

**Windows:**
```bash
cd src
build.bat
```

**Linux/macOS:**
```bash
cd src
chmod +x build.sh
./build.sh
```

#### 3. Configuration
```bash
# Copy example configuration
cp config/llm_config.conf.example config/llm_config.conf

# Edit with your API keys
notepad config/llm_config.conf  # Windows
nano config/llm_config.conf     # Linux/macOS
```

## 📖 Usage Examples

### Document Analysis Pipeline
```bash
# 1. Download SEC filings
./sec_data_acquisition.exe download-company 0000320193 10-K 10-Q

# 2. Parse documents
./document_parser.exe data/sec_filings/AAPL_10K_2023.txt output/parsed.json

# 3. Generate embeddings
./tfidf_embedding.exe data/ output/embeddings.json

# 4. Query system
./query_engine.exe "revenue growth trends" --top 5 --output results.json
```

### RAG-based Financial Analysis
```bash
# Comprehensive analysis
./rag_engine.exe analyze \
  --query "What are Apple's key financial performance indicators?" \
  --context data/apple_10k_2023_sample.txt \
  --output analysis_report.md

# Financial metrics extraction
./financial_analyzer.exe \
  --input data/microsoft_10k_2024_sample.txt \
  --metrics revenue,profit,debt \
  --format json
```

### Unified CLI Interface
```bash
# Get help
./main.exe help

# Run complete analysis pipeline
./main.exe analyze --company AAPL --years 2023,2024 --output comprehensive_report.json

# Interactive query mode
./main.exe query --interactive
```

## 🧪 Testing

### Automated Testing
```bash
# Run all tests
cd tests && chmod +x run_tests.sh && ./run_tests.sh

# Unit tests only
cd tests/unit
g++ -std=c++17 -I"../../src" test_*.cpp -o test_runner && ./test_runner

# Integration tests
cd tests/integration
# Note: Integration tests require refactoring for current architecture
```

### Manual Testing
```bash
# Test document parsing
echo "Sample financial text" > test.txt
./document_parser.exe test.txt output.json

# Test TF-IDF system
./tfidf_embedding.exe help

# Test query system
./query_engine.exe "test query" --dry-run
```

## 🌐 Web Demo

Launch the interactive web interface:

```bash
cd web_demo
python app.py
# Open http://localhost:5000 in your browser
```

Features:
- Document upload and parsing
- Real-time query processing
- Visual results display
- Export capabilities

## 🔧 Development

### Branch Structure
- **main**: Production branch (protected, stable releases)
- **master**: Development branch (active development)

### Development Workflow
```bash
# Start development
git checkout master
git pull origin master

# Make changes, test, commit
git add .
git commit -m "feat: add new feature"
git push origin master

# Create production release via PR: master → main
```

### CI/CD Pipeline
- **Automated Testing**: Unit, integration, and build tests
- **Multi-Platform**: Linux and Windows builds
- **Quality Gates**: Code review required for production
- **Automated Deployment**: Releases created on main branch updates

See [DEVELOPMENT.md](DEVELOPMENT.md) for detailed workflow guide.

## 📊 Project Status

### Current Sprint: Production Deployment
- ✅ Core system architecture complete
- ✅ All major components implemented
- ✅ SEC EDGAR integration functional
- ✅ Web demo operational
- ✅ CI/CD pipeline established
- ✅ Documentation comprehensive
- 🔄 Performance optimization ongoing
- 🔄 Advanced analytics features in development

### Recent Updates
- Added automated CI/CD pipeline with GitHub Actions
- Implemented branch protection and professional Git workflow
- Enhanced documentation with development guides
- Improved error handling and logging
- Added comprehensive test coverage

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guidelines](DEVELOPMENT.md#contributing).

1. **Fork** the repository
2. **Create** a feature branch from `master`
3. **Make** your changes with tests
4. **Submit** a pull request to `master`
5. **Address** review feedback
6. **Merge** after approval

## 📚 Documentation

### Setup Guides
- [📋 Installation Guide](docs/INSTALLATION.md)
- [🪟 Windows Setup](docs/WINDOWS_SETUP.md)
- [🔧 Development Workflow](DEVELOPMENT.md)

### Technical Documentation
- [🏛️ SEC API Research](docs/sec_api_research.md)
- [🤖 LLM Integration](docs/openrouter_research.md)
- [🧠 ONNX Research](docs/onnx_research.md)

### Project Reports
- [📈 Sprint Reports](docs/) - Development progress tracking
- [🎯 Project Summary](PROJECT_SUMMARY.md) - High-level overview

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🆘 Support

- **Issues**: [GitHub Issues](https://github.com/p-kowadkar/financial-doc-analysis/issues)
- **Discussions**: [GitHub Discussions](https://github.com/p-kowadkar/financial-doc-analysis/discussions)
- **Documentation**: Check the `docs/` directory
- **Examples**: See `data/` directory for sample files

## 🏆 Acknowledgments

- SEC EDGAR API for financial data access
- OpenRouter for LLM integration capabilities
- Contributors and testers who helped improve the system

---

**⭐ Star this repository if you find it useful!**
