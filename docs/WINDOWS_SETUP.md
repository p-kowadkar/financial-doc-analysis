# Windows Installation & Setup Guide

## Step-by-Step Installation

### Method 1: MinGW-w64 (Recommended for Beginners)

1. **Download MinGW-w64**
   - Go to: https://www.mingw-w64.org/downloads/
   - Click "MingW-W64-builds"
   - Download the installer

2. **Install MinGW-w64**
   - Run the installer
   - Choose these settings:
     - Version: Latest
     - Architecture: x86_64
     - Threads: posix
     - Exception: seh
     - Build revision: Latest
   - Install to: `C:\mingw64`

3. **Add to PATH**
   - Open "Environment Variables" in Windows
   - Add `C:\mingw64\bin` to your PATH
   - Restart Command Prompt

4. **Verify Installation**
   ```cmd
   g++ --version
   ```
   Should show version information.

### Method 2: Visual Studio Community

1. **Download Visual Studio Community**
   - Go to: https://visualstudio.microsoft.com/vs/community/
   - Download and install (free)

2. **Select Workloads**
   - Choose "Desktop development with C++"
   - Install

3. **Use Developer Command Prompt**
   - Search for "Developer Command Prompt for VS"
   - Use `cl` instead of `g++` for compilation

### Method 3: MSYS2 (Advanced Users)

1. **Download MSYS2**
   - Go to: https://www.msys2.org/
   - Download and install

2. **Install GCC**
   ```bash
   pacman -S mingw-w64-x86_64-gcc
   pacman -S mingw-w64-x86_64-make
   ```

3. **Add to PATH**
   - Add `C:\msys64\mingw64\bin` to PATH

## Quick Build & Test

1. **Extract the project**
2. **Open Command Prompt**
3. **Navigate to src directory**
   ```cmd
   cd path\to\financial-doc-analysis-windows\src
   ```

4. **Run build script**
   ```cmd
   build.bat
   ```

5. **Test the system**
   ```cmd
   main.exe help
   main.exe build-index
   main.exe query "financial risks" --top 3
   ```

## Troubleshooting

### "g++ is not recognized"
- MinGW-w64 not installed or not in PATH
- Restart Command Prompt after adding to PATH
- Try full path: `C:\mingw64\bin\g++.exe --version`

### "Permission denied"
- Run Command Prompt as Administrator
- Check antivirus software
- Disable real-time protection temporarily

### Build errors
- Ensure C++17 support: `g++ -std=c++17 --version`
- Update to latest MinGW-w64
- Try individual compilation:
  ```cmd
  g++ -std=c++17 -o test.exe main.cpp
  ```

### File not found errors
- Check current directory: `dir`
- Ensure data files exist: `dir ..\data`
- Use absolute paths if needed

## Alternative Compilers

### Using MSVC (Visual Studio)
```cmd
cl /std:c++17 /EHsc main.cpp /Fe:main.exe
```

### Using Clang++
```cmd
clang++ -std=c++17 -o main.exe main.cpp
```

## Performance Tips

1. **Use Release Build**
   ```cmd
   g++ -std=c++17 -O2 -DNDEBUG -o main.exe main.cpp
   ```

2. **Enable All Optimizations**
   ```cmd
   g++ -std=c++17 -O3 -march=native -o main.exe main.cpp
   ```

3. **Static Linking (Portable)**
   ```cmd
   g++ -std=c++17 -O2 -static -o main.exe main.cpp
   ```

## Success Indicators

After successful build, you should see:
- 7 executable files in src/ directory
- No compilation errors
- `main.exe help` shows usage information
- `main.exe build-index` completes without errors

## Next Steps

Once installed successfully:
1. Run `main.exe build-index` to create the document index
2. Try sample queries: `main.exe query "cybersecurity" --top 3`
3. Generate analysis: `main.exe analyze`
4. Check output files in `../output/` directory

