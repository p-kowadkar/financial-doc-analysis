# Installation Guide: C++ Financial Document Analysis System

This guide provides detailed installation instructions for all supported platforms: Linux, macOS, and Windows.

## 📋 System Requirements

### Minimum Requirements
- **CPU**: x86_64 architecture (Intel/AMD 64-bit)
- **RAM**: 4GB minimum, 8GB recommended
- **Storage**: 2GB free space for source code and dependencies
- **Compiler**: C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)

### Recommended Requirements
- **CPU**: Multi-core processor (4+ cores)
- **RAM**: 16GB for large document processing
- **Storage**: 10GB for extensive document collections
- **Network**: Stable internet connection for SEC EDGAR API and LLM services

## 🐧 Linux Installation

### Ubuntu/Debian

#### Step 1: Update System
```bash
sudo apt update && sudo apt upgrade -y
```

#### Step 2: Install Build Tools
```bash
# Essential build tools
sudo apt install -y build-essential cmake git wget curl

# Additional development tools
sudo apt install -y pkg-config autoconf automake libtool
```

#### Step 3: Install Optional Dependencies

**For Neural Embeddings (ONNX Runtime):**
```bash
# Option 1: Install from package manager (Ubuntu 20.04+)
sudo apt install -y libonnxruntime-dev

# Option 2: Download and install manually
wget https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-linux-x64-1.16.3.tgz
tar -xzf onnxruntime-linux-x64-1.16.3.tgz
sudo cp -r onnxruntime-linux-x64-1.16.3/include/* /usr/local/include/
sudo cp -r onnxruntime-linux-x64-1.16.3/lib/* /usr/local/lib/
sudo ldconfig
```

**For LLM Integration (libcurl):**
```bash
sudo apt install -y libcurl4-openssl-dev libssl-dev
```

**For JSON Processing (nlohmann-json):**
```bash
sudo apt install -y nlohmann-json3-dev
```

#### Step 4: Build the System
```bash
# Clone repository
git clone https://github.com/your-repo/financial-doc-analysis.git
cd financial-doc-analysis

# Option 1: Use build script (recommended)
cd src
chmod +x build.sh
./build.sh

# Option 2: Use CMake
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Option 3: Manual compilation (no dependencies)
cd src
g++ -std=c++17 -O2 -o main main.cpp
g++ -std=c++17 -O2 -o document_parser document_parser.cpp
g++ -std=c++17 -O2 -o tfidf_embedding tfidf_embedding.cpp
g++ -std=c++17 -O2 -o financial_analyzer financial_analyzer.cpp
g++ -std=c++17 -O2 -o query_engine query_engine.cpp
g++ -std=c++17 -O2 -o rag_engine rag_engine.cpp

# With dependencies (if installed)
g++ -std=c++17 -O2 -lcurl -o sec_data_acquisition sec_data_acquisition.cpp
g++ -std=c++17 -O2 -lonnxruntime -o neural_embeddings neural_embeddings.cpp
```

#### Step 5: Verify Installation
```bash
# Test basic functionality
./main help
./document_parser --version
./tfidf_embedding --help

# Run test suite
cd ../tests
./run_tests.sh
```

### CentOS/RHEL/Fedora

#### Step 1: Install Build Tools
```bash
# CentOS/RHEL 8+
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake git wget curl

# CentOS/RHEL 7
sudo yum groupinstall "Development Tools"
sudo yum install cmake3 git wget curl
```

#### Step 2: Install Dependencies
```bash
# Fedora
sudo dnf install libcurl-devel openssl-devel json-devel

# CentOS/RHEL
sudo dnf install libcurl-devel openssl-devel
# Note: nlohmann-json may need manual installation
```

#### Step 3: Build (same as Ubuntu)
Follow the same build steps as Ubuntu/Debian.

### Arch Linux

#### Step 1: Install Dependencies
```bash
sudo pacman -S base-devel cmake git wget curl
sudo pacman -S curl openssl nlohmann-json

# Optional: ONNX Runtime
yay -S onnxruntime  # Using AUR helper
```

#### Step 2: Build (same as Ubuntu)
Follow the same build steps as Ubuntu/Debian.

## 🍎 macOS Installation

### Prerequisites

#### Step 1: Install Xcode Command Line Tools
```bash
xcode-select --install
```

#### Step 2: Install Homebrew (if not already installed)
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

#### Step 3: Install Dependencies
```bash
# Essential tools
brew install cmake git wget curl

# Optional dependencies
brew install nlohmann-json
brew install curl  # Usually pre-installed
brew install openssl

# Optional: ONNX Runtime
brew install onnxruntime
```

### Build the System

#### Option 1: Build Script (Recommended)
```bash
git clone https://github.com/your-repo/financial-doc-analysis.git
cd financial-doc-analysis/src
chmod +x build.sh
./build.sh
```

#### Option 2: CMake
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
```

#### Option 3: Manual Compilation
```bash
cd src

# Basic compilation (no external dependencies)
clang++ -std=c++17 -O2 -o main main.cpp
clang++ -std=c++17 -O2 -o document_parser document_parser.cpp
clang++ -std=c++17 -O2 -o tfidf_embedding tfidf_embedding.cpp
clang++ -std=c++17 -O2 -o financial_analyzer financial_analyzer.cpp
clang++ -std=c++17 -O2 -o query_engine query_engine.cpp
clang++ -std=c++17 -O2 -o rag_engine rag_engine.cpp

# With dependencies (if installed via Homebrew)
clang++ -std=c++17 -O2 -I/opt/homebrew/include -L/opt/homebrew/lib -lcurl -o sec_data_acquisition sec_data_acquisition.cpp
```

### Troubleshooting macOS

#### Apple Silicon (M1/M2) Considerations
```bash
# If using Homebrew on Apple Silicon
export CPPFLAGS="-I/opt/homebrew/include"
export LDFLAGS="-L/opt/homebrew/lib"

# For Intel Macs
export CPPFLAGS="-I/usr/local/include"
export LDFLAGS="-L/usr/local/lib"
```

#### OpenSSL Issues
```bash
# If OpenSSL linking issues occur
export LDFLAGS="-L/opt/homebrew/opt/openssl@3/lib"
export CPPFLAGS="-I/opt/homebrew/opt/openssl@3/include"
```

## 🪟 Windows Installation

### Option 1: MinGW-w64 (Recommended)

#### Step 1: Install MinGW-w64
1. Download [MinGW-w64](https://www.mingw-w64.org/downloads/)
2. Choose "MingW-W64-builds" installer
3. Install with these settings:
   - Version: Latest
   - Architecture: x86_64
   - Threads: posix
   - Exception: seh
   - Build revision: Latest

#### Step 2: Add to PATH
1. Add MinGW-w64 bin directory to system PATH
   - Default: `C:\mingw64\bin`
2. Verify installation:
```cmd
g++ --version
```

#### Step 3: Install Git (if not already installed)
Download and install [Git for Windows](https://git-scm.com/download/win)

#### Step 4: Build the System
```cmd
REM Clone repository
git clone https://github.com/your-repo/financial-doc-analysis.git
cd financial-doc-analysis\src

REM Build using batch script
build.bat

REM Or build manually
g++ -std=c++17 -O2 -o main.exe main.cpp
g++ -std=c++17 -O2 -o document_parser.exe document_parser.cpp
g++ -std=c++17 -O2 -o tfidf_embedding.exe tfidf_embedding.cpp
g++ -std=c++17 -O2 -o financial_analyzer.exe financial_analyzer.cpp
g++ -std=c++17 -O2 -o query_engine.exe query_engine.cpp
g++ -std=c++17 -O2 -o rag_engine.exe rag_engine.cpp
```

### Option 2: Visual Studio

#### Step 1: Install Visual Studio
1. Download [Visual Studio Community 2022](https://visualstudio.microsoft.com/downloads/)
2. During installation, select:
   - "Desktop development with C++"
   - Windows 10/11 SDK
   - CMake tools for C++

#### Step 2: Build with Visual Studio
```cmd
REM Open Developer Command Prompt for VS 2022
REM Navigate to project directory
cd financial-doc-analysis

REM Build with CMake
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

#### Step 3: Build with MSBuild
```cmd
REM In Developer Command Prompt
cd src
cl /std:c++17 /O2 /EHsc main.cpp /Fe:main.exe
cl /std:c++17 /O2 /EHsc document_parser.cpp /Fe:document_parser.exe
cl /std:c++17 /O2 /EHsc tfidf_embedding.cpp /Fe:tfidf_embedding.exe
cl /std:c++17 /O2 /EHsc financial_analyzer.cpp /Fe:financial_analyzer.exe
cl /std:c++17 /O2 /EHsc query_engine.cpp /Fe:query_engine.exe
cl /std:c++17 /O2 /EHsc rag_engine.cpp /Fe:rag_engine.exe
```

### Option 3: WSL (Windows Subsystem for Linux)

#### Step 1: Install WSL2
```powershell
# Run in PowerShell as Administrator
wsl --install
# Restart computer when prompted
```

#### Step 2: Install Ubuntu
```powershell
wsl --install -d Ubuntu
```

#### Step 3: Follow Linux Instructions
Once in WSL Ubuntu, follow the Ubuntu/Debian installation instructions above.

### Windows Dependencies

#### Installing libcurl on Windows
```cmd
REM Option 1: Use vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install curl:x64-windows

REM Option 2: Download pre-built binaries
REM Download from https://curl.se/windows/
REM Extract and add to include/lib paths
```

#### Installing ONNX Runtime on Windows
```cmd
REM Download from GitHub releases
REM https://github.com/microsoft/onnxruntime/releases
REM Extract and add to include/lib paths
```

## 🔧 Configuration

### Environment Variables
```bash
# Optional: Set custom paths
export FINANCIAL_DOC_ANALYSIS_HOME=/path/to/installation
export FINANCIAL_DOC_ANALYSIS_DATA=/path/to/data
export FINANCIAL_DOC_ANALYSIS_CONFIG=/path/to/config

# For neural embeddings
export ONNX_MODEL_PATH=/path/to/models
export ONNX_PROVIDERS=CPUExecutionProvider

# For LLM integration
export OPENROUTER_API_KEY=your_api_key_here
export LLM_CACHE_DIR=/path/to/cache
```

### Configuration Files
```bash
# Copy default configurations
cp config/sec_config.conf.example config/sec_config.conf
cp config/llm_config.conf.example config/llm_config.conf
cp config/neural_config.conf.example config/neural_config.conf

# Edit configurations as needed
nano config/sec_config.conf
```

## ✅ Verification

### Basic Functionality Test
```bash
# Test core components
./main help
./document_parser --version
./tfidf_embedding --help

# Test with sample data
./main build-index
./main query "test query" --top 3
```

### Comprehensive Test Suite
```bash
cd tests
./run_tests.sh
```

Expected output:
```
==========================================
C++ Financial Document Analysis System
Comprehensive Test Suite
==========================================

Total Tests: 28
Passed: 28
Failed: 0
Success Rate: 100%

[SUCCESS] ALL TESTS PASSED! 🎉
```

### Performance Benchmark
```bash
cd tests/performance
./run_benchmarks.sh
```

## 🐛 Troubleshooting

### Common Issues

#### Compilation Errors
```bash
# Missing C++17 support
# Solution: Update compiler or use -std=c++17 flag

# Missing headers
# Solution: Install development packages
sudo apt install build-essential  # Linux
brew install cmake  # macOS
```

#### Runtime Errors
```bash
# Library not found
# Solution: Check LD_LIBRARY_PATH (Linux) or DYLD_LIBRARY_PATH (macOS)
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# Permission denied
# Solution: Make executables executable
chmod +x main document_parser tfidf_embedding
```

#### Memory Issues
```bash
# Out of memory during large document processing
# Solution: Increase system memory or process documents in batches
./main query "test" --batch-size 100
```

### Platform-Specific Issues

#### Linux
- **Issue**: `libcurl.so.4: cannot open shared object file`
- **Solution**: `sudo apt install libcurl4-openssl-dev`

#### macOS
- **Issue**: `dyld: Library not loaded: @rpath/libcurl.4.dylib`
- **Solution**: `brew install curl` and check library paths

#### Windows
- **Issue**: `'g++' is not recognized as an internal or external command`
- **Solution**: Add MinGW-w64 bin directory to PATH

### Getting Help

1. **Check Documentation**: Review relevant sections in `docs/`
2. **Search Issues**: Look for similar problems in GitHub Issues
3. **Run Diagnostics**: Use `./main diagnose` for system information
4. **Contact Support**: Create a GitHub Issue with:
   - Operating system and version
   - Compiler version
   - Complete error message
   - Steps to reproduce

## 📚 Next Steps

After successful installation:

1. **Read the User Guide**: `docs/user_guide.md`
2. **Try Examples**: `examples/` directory
3. **Configure APIs**: Set up SEC EDGAR and LLM API keys
4. **Run Tutorials**: Follow `docs/tutorials/`
5. **Join Community**: GitHub Discussions for questions and tips

---

**Installation complete! You're ready to analyze financial documents with the C++ Financial Document Analysis System.**

