#!/bin/bash
# Alternative build script for Unix-like systems (Linux, macOS, WSL)

echo "Building C++ Financial Document Analysis System..."
echo

# Check if g++ is available
if ! command -v g++ &> /dev/null; then
    echo "Error: g++ compiler not found. Please install build-essential or equivalent."
    exit 1
fi

echo "Compiling components..."

# Compile all components
echo "- Compiling html_text_extractor.cpp..."
g++ -std=c++17 -O2 -o html_text_extractor html_text_extractor.cpp
if [ $? -ne 0 ]; then
    echo "Error compiling html_text_extractor.cpp"
    exit 1
fi

echo "- Compiling document_parser.cpp..."
g++ -std=c++17 -O2 -o document_parser document_parser.cpp
if [ $? -ne 0 ]; then
    echo "Error compiling document_parser.cpp"
    exit 1
fi

echo "- Compiling tfidf_embedding.cpp..."
g++ -std=c++17 -O2 -o tfidf_embedding tfidf_embedding.cpp
if [ $? -ne 0 ]; then
    echo "Error compiling tfidf_embedding.cpp"
    exit 1
fi

echo "- Compiling query_engine.cpp..."
g++ -std=c++17 -O2 -o query_engine query_engine.cpp
if [ $? -ne 0 ]; then
    echo "Error compiling query_engine.cpp"
    exit 1
fi

echo "- Compiling rag_engine.cpp..."
g++ -std=c++17 -O2 -o rag_engine rag_engine.cpp
if [ $? -ne 0 ]; then
    echo "Error compiling rag_engine.cpp"
    exit 1
fi

echo "- Compiling financial_analyzer.cpp..."
g++ -std=c++17 -O2 -o financial_analyzer financial_analyzer.cpp
if [ $? -ne 0 ]; then
    echo "Error compiling financial_analyzer.cpp"
    exit 1
fi

echo "- Compiling main.cpp..."
g++ -std=c++17 -O2 -o main main.cpp
if [ $? -ne 0 ]; then
    echo "Error compiling main.cpp"
    exit 1
fi

# Compile SEC data acquisition tool (if libcurl is available)
echo "- Compiling sec_data_acquisition.cpp..."
if pkg-config --exists libcurl 2>/dev/null; then
    echo "  Found libcurl, compiling with network support..."
    g++ -std=c++17 -O2 $(pkg-config --cflags libcurl) $(pkg-config --libs libcurl) -o sec_data_acquisition sec_data_acquisition.cpp sec_edgar_client.cpp
    if [ $? -ne 0 ]; then
        echo "Warning: Failed to compile sec_data_acquisition.cpp with libcurl"
    fi
else
    echo "  libcurl not found, skipping sec_data_acquisition compilation"
    echo "  To install libcurl: sudo apt install libcurl4-openssl-dev (Ubuntu/Debian)"
fi

echo
echo "Build completed successfully!"
echo
echo "Available executables:"
echo "- html_text_extractor"
echo "- document_parser"
echo "- tfidf_embedding"
echo "- query_engine"
echo "- rag_engine"
echo "- financial_analyzer"
echo "- main (unified CLI interface)"
if [ -f "sec_data_acquisition" ]; then
    echo "- sec_data_acquisition (SEC EDGAR data downloader)"
fi
echo
echo "To get started, run: ./main help"
if [ -f "sec_data_acquisition" ]; then
    echo "For SEC data: ./sec_data_acquisition help"
fi
echo

