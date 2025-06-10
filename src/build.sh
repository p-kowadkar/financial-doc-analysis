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

# Compile core components (no external dependencies)
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

# Try to compile components that require libcurl (optional)
echo "- Attempting to compile libcurl-dependent components..."
SEC_COMPILED=false
LLM_COMPILED=false

if pkg-config --exists libcurl 2>/dev/null; then
    echo "  Found libcurl via pkg-config"
    CURL_CFLAGS=$(pkg-config --cflags libcurl)
    CURL_LIBS=$(pkg-config --libs libcurl)
    
    echo "  - Compiling sec_data_acquisition.cpp..."
    g++ -std=c++17 -O2 $CURL_CFLAGS -o sec_data_acquisition sec_data_acquisition.cpp sec_edgar_client.cpp $CURL_LIBS 2>/dev/null
    if [ $? -eq 0 ]; then
        echo "    ✓ sec_data_acquisition compiled successfully"
        SEC_COMPILED=true
    else
        echo "    ✗ sec_data_acquisition compilation failed"
    fi
    
    echo "  - Testing llm_client.cpp compilation..."
    g++ -std=c++17 -O2 $CURL_CFLAGS -c llm_client.cpp $CURL_LIBS 2>/dev/null
    if [ $? -eq 0 ]; then
        echo "    ✓ llm_client.cpp compiles successfully"
        rm -f llm_client.o  # Clean up object file
        LLM_COMPILED=true
    else
        echo "    ✗ llm_client.cpp compilation failed"
    fi
else
    echo "  ✗ libcurl not found via pkg-config"
    echo "    To install: sudo apt install libcurl4-openssl-dev pkg-config"
fi

echo
echo "Build completed successfully!"
echo
echo "Core executables (always available):"
echo "- html_text_extractor"
echo "- document_parser"
echo "- tfidf_embedding"
echo "- query_engine"
echo "- rag_engine"
echo "- financial_analyzer"
echo "- main (unified CLI interface)"

echo
echo "Optional components (require libcurl):"
if [ "$SEC_COMPILED" = true ]; then
    echo "✓ sec_data_acquisition - Available"
else
    echo "✗ sec_data_acquisition - Not available"
fi

if [ "$LLM_COMPILED" = true ]; then
    echo "✓ llm_client - Available for compilation"
else
    echo "✗ llm_client - Not available"
fi

echo
echo "To get started, run: ./main help"
if [ "$SEC_COMPILED" = true ]; then
    echo "For SEC data: ./sec_data_acquisition help"
fi
echo
