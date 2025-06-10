@echo off
REM Windows Build Script for C++ Financial Document Analysis System

echo Building C++ Financial Document Analysis System for Windows...
echo.

REM Check if g++ is available
g++ --version >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: g++ compiler not found. Please install MinGW-w64 or MSYS2.
    echo Download from: https://www.mingw-w64.org/downloads/
    pause
    exit /b 1
)

REM Check for curl
where curl >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: curl not found. Please install MSYS2 and run:
    echo pacman -S mingw-w64-x86_64-curl
    pause
    exit /b 1
)

REM Set include and library paths for curl
set CURL_INCLUDE=C:\msys64\mingw64\include
set CURL_LIB=C:\msys64\mingw64\lib

echo Compiling components...

REM Compile HTML text extractor
echo - Compiling html_text_extractor.cpp...
g++ -std=c++17 -O2 -o html_text_extractor.exe html_text_extractor.cpp
if %errorlevel% neq 0 (
    echo Error compiling html_text_extractor.cpp
    pause
    exit /b 1
)

REM Compile SEC data acquisition components
echo - Compiling SEC data acquisition components...
g++ -std=c++17 -O2 -I"%CURL_INCLUDE%" -o sec_data_acquisition.exe sec_data_acquisition.cpp sec_edgar_client.cpp -L"%CURL_LIB%" -lcurl
if %errorlevel% neq 0 (
    echo Error compiling SEC data acquisition components
    pause
    exit /b 1
)

REM Compile document parser
echo - Compiling document_parser.cpp...
g++ -std=c++17 -O2 -o document_parser.exe document_parser.cpp
if %errorlevel% neq 0 (
    echo Error compiling document_parser.cpp
    pause
    exit /b 1
)

REM Compile TF-IDF embedding system
echo - Compiling tfidf_embedding.cpp...
g++ -std=c++17 -O2 -o tfidf_embedding.exe tfidf_embedding.cpp
if %errorlevel% neq 0 (
    echo Error compiling tfidf_embedding.cpp
    pause
    exit /b 1
)

REM Compile query engine
echo - Compiling query_engine.cpp...
g++ -std=c++17 -O2 -o query_engine.exe query_engine.cpp
if %errorlevel% neq 0 (
    echo Error compiling query_engine.cpp
    pause
    exit /b 1
)

REM Compile RAG engine
echo - Compiling rag_engine.cpp...
g++ -std=c++17 -O2 -o rag_engine.exe rag_engine.cpp
if %errorlevel% neq 0 (
    echo Error compiling rag_engine.cpp
    pause
    exit /b 1
)

REM Compile financial analyzer
echo - Compiling financial_analyzer.cpp...
g++ -std=c++17 -O2 -o financial_analyzer.exe financial_analyzer.cpp
if %errorlevel% neq 0 (
    echo Error compiling financial_analyzer.cpp
    pause
    exit /b 1
)

REM Compile LLM client (if libcurl is available)
echo - Compiling llm_client.cpp...
if exist "C:\Users\Asus\vcpkg\installed\x64-windows\include\curl\curl.h" (
    echo   Found libcurl, compiling LLM client...
    g++ -std=c++17 -O2 -I"C:\Users\Asus\vcpkg\installed\x64-windows\include" -c llm_client.cpp
    if %errorlevel% neq 0 (
        echo Warning: Failed to compile llm_client.cpp
        set LLM_SUPPORT=false
    ) else (
        set LLM_SUPPORT=true
    )
) else (
    echo   libcurl not found, compiling without LLM support
    set LLM_SUPPORT=false
)

REM Compile main CLI interface
echo - Compiling main.cpp...
if "%LLM_SUPPORT%"=="true" (
    echo   Compiling with LLM support...
    g++ -std=c++17 -O2 -I"C:\Users\Asus\vcpkg\installed\x64-windows\include" -L"C:\Users\Asus\vcpkg\installed\x64-windows\bin" -o main.exe main.cpp llm_client.o -lcurl -lws2_32 -lwldap32 -lcrypt32 -lnormaliz
) else (
    echo   Compiling without LLM support...
    g++ -std=c++17 -O2 -o main.exe main.cpp
)
if %errorlevel% neq 0 (
    echo Error compiling main.cpp
    pause
    exit /b 1
)

REM Compile SEC data acquisition tool (if libcurl is available)
echo - Compiling sec_data_acquisition.cpp...
if exist "C:\Users\Asus\vcpkg\installed\x64-windows\include\curl\curl.h" (
    echo   Found libcurl, compiling with network support...
    g++ -std=c++17 -O2 -I"C:\Users\Asus\vcpkg\installed\x64-windows\include" -L"C:\Users\Asus\vcpkg\installed\x64-windows\bin" -lcurl -lws2_32 -lwldap32 -lcrypt32 -lnormaliz -o sec_data_acquisition.exe sec_data_acquisition.cpp sec_edgar_client.cpp
    if %errorlevel% neq 0 (
        echo Warning: Failed to compile sec_data_acquisition.cpp with libcurl
    ) else (
        echo   Copying required DLLs...
        copy "C:\Users\Asus\vcpkg\installed\x64-windows\bin\libcurl.dll" . >nul 2>&1
        copy "C:\Users\Asus\vcpkg\installed\x64-windows\bin\zlib1.dll" . >nul 2>&1
    )
) else (
    echo   libcurl not found, skipping sec_data_acquisition compilation
    echo   To install libcurl: cd C:\Users\Asus\vcpkg && .\vcpkg install curl:x64-windows
)

echo.
echo Build completed successfully!
echo.
echo Available executables:
echo - html_text_extractor.exe
echo - document_parser.exe
echo - tfidf_embedding.exe
echo - query_engine.exe
echo - rag_engine.exe
echo - financial_analyzer.exe
echo - main.exe (unified CLI interface)
if exist sec_data_acquisition.exe (
    echo - sec_data_acquisition.exe (SEC EDGAR data downloader)
)
echo.
echo To get started, run: main.exe help
if exist sec_data_acquisition.exe (
    echo For SEC data: sec_data_acquisition.exe help
)
echo.
pause

