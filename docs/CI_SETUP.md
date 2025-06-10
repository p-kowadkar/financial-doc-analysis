# CI/CD Pipeline Setup and Testing Guide

## Overview
This document outlines the CI/CD pipeline configuration and testing improvements implemented for the Financial Document Analysis System.

## Pipeline Components

### 1. Unit Tests
All unit tests have been simplified and made self-contained:
- `test_document_parser.cpp`: Tests document parsing functionality
- `test_tfidf_embedding.cpp`: Tests TF-IDF embedding and similarity search
- `test_llm_client.cpp`: Tests LLM client integration

### 2. Build Workflows
Two main build workflows are configured:

#### Linux Build (Ubuntu)
```yaml
- Install dependencies (g++, libcurl)
- Create necessary directories
- Build all components
- Run unit tests
- Test executable functionality
```

#### Windows Build (MSYS2/MinGW)
```yaml
- Setup MinGW environment
- Create necessary directories
- Build all components
- Test executable functionality
```

### 3. Deployment
Automated deployment on main branch:
- Creates GitHub release
- Uploads build artifacts
- Tags with version number

## Local Testing

### Running Unit Tests
```bash
# Document Parser Test
cd tests/unit
g++ -std=c++17 test_document_parser.cpp -o test_document_parser
./test_document_parser

# TF-IDF Embedding Test
g++ -std=c++17 test_tfidf_embedding.cpp -o test_tfidf_embedding
./test_tfidf_embedding

# LLM Client Test
g++ -std=c++17 test_llm_client.cpp -o test_llm_client
./test_llm_client
```

### Building Components
```bash
# On Windows (using MSYS2)
cd src
g++ -std=c++17 -O2 -o document_parser.exe document_parser.cpp
g++ -std=c++17 -O2 -o html_text_extractor.exe html_text_extractor.cpp
# ... (repeat for other components)

# On Linux
cd src
chmod +x build.sh
./build.sh
```

## CI/CD Pipeline Status

### Current Status
- ✅ Unit tests passing locally
- ✅ Build scripts working on both platforms
- ✅ Deployment configuration complete

### Monitoring CI/CD
To monitor the CI/CD pipeline:
1. Go to your repository on GitHub
2. Click on "Actions" tab
3. View the latest workflow runs
4. Check detailed logs for any failures

### Common Issues and Solutions

#### Test Failures
- Check test output for specific failure points
- Verify all dependencies are installed
- Ensure file paths are correct for both Windows and Linux

#### Build Failures
- Verify compiler version (requires C++17)
- Check MSYS2 installation on Windows
- Ensure all source files are present

#### Deployment Issues
- Verify GitHub token permissions
- Check branch protection rules
- Ensure version tagging is correct

## Next Steps

1. Monitor the current CI run for the simplified tests
2. Add more comprehensive integration tests
3. Implement code coverage reporting
4. Add static analysis checks
5. Configure automated dependency updates

## Best Practices

1. Always run tests locally before pushing
2. Keep unit tests focused and independent
3. Maintain consistent file paths across platforms
4. Use relative paths in build scripts
5. Document any new CI/CD changes

## Support

For CI/CD related issues:
1. Check GitHub Actions logs
2. Review this documentation
3. Consult the team lead
4. Open an issue if needed

Remember to keep this document updated as the CI/CD pipeline evolves.
