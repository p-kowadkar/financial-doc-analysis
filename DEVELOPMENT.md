# Development Workflow Guide

This document outlines the development workflow for the Financial Document Analysis System.

## Branch Structure

- **main**: Production branch (protected, stable releases only)
- **master**: Development branch (active development, testing)

## Development Process

### 1. Daily Development
Work on the `master` branch for all development activities:

```bash
git checkout master
git pull origin master

# Make your changes
# ... code, test, commit ...

git add .
git commit -m "feat: add new feature"
git push origin master
```

### 2. Creating a Release
When ready to deploy to production:

```bash
# Ensure you're on master with latest changes
git checkout master
git pull origin master

# Create a pull request from master to main
# This triggers code review and CI/CD pipeline
```

### 3. Pull Request Process
1. Create PR from `master` → `main`
2. Automated tests run via GitHub Actions
3. Code review by team members
4. Merge to `main` triggers production deployment

## CI/CD Pipeline

### Automated Testing
- **Unit Tests**: Test individual components
- **Integration Tests**: Test component interactions
- **Build Tests**: Verify compilation on multiple platforms

### Deployment
- **Development**: Automatic deployment on `master` push
- **Production**: Manual deployment after PR approval to `main`

## Local Development Setup

### Prerequisites
```bash
# Install dependencies
vcpkg install curl:x64-windows

# Build project
cd src
build.bat  # Windows
./build.sh # Unix
```

### Testing
```bash
# Run unit tests
cd tests/unit
g++ -std=c++17 -I"../../src" test_*.cpp -o test_runner
./test_runner

# Run integration tests
cd tests/integration
# Note: Integration tests need refactoring due to compilation issues
```

### Code Quality
- Follow C++17 standards
- Use meaningful commit messages
- Add tests for new features
- Update documentation

## Environment Configuration

### Development Environment
- Use `config/llm_config.conf` for local testing
- Set environment variables for API keys
- Use sample data in `data/` directory

### Production Environment
- Environment variables managed via GitHub Secrets
- Automated configuration validation
- Monitoring and logging enabled

## Common Commands

### Git Workflow
```bash
# Start new feature
git checkout master
git pull origin master
git checkout -b feature/new-feature

# Commit changes
git add .
git commit -m "feat: implement new feature"

# Push to master (after testing)
git checkout master
git merge feature/new-feature
git push origin master

# Create production release
# Use GitHub UI to create PR: master → main
```

### Build and Test
```bash
# Full build
cd src && build.bat

# Quick test
main.exe help

# Run analysis
main.exe analyze

# Test query
main.exe query "financial performance" --top 5
```

### Debugging
```bash
# Verbose logging
main.exe query "test" --verbose

# Check configuration
main.exe config --validate

# Test API connectivity
main.exe llm "test" --dry-run
```

## Troubleshooting

### Common Issues
1. **Build Failures**: Check compiler version and dependencies
2. **API Errors**: Verify API keys and network connectivity
3. **Test Failures**: Ensure sample data is available

### Getting Help
- Check existing issues on GitHub
- Review documentation in `docs/` directory
- Contact maintainers via GitHub discussions

## Contributing

1. Fork the repository
2. Create feature branch from `master`
3. Make changes with tests
4. Submit pull request to `master`
5. Address review feedback
6. Merge after approval

## Release Notes

Track changes in each release:
- New features
- Bug fixes
- Breaking changes
- Performance improvements

See [Releases](https://github.com/p-kowadkar/financial-doc-analysis/releases) for version history.
