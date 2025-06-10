# ONNX Runtime C++ Integration Research

## Key Findings

### Available Builds
1. **Microsoft.ML.OnnxRuntime** - CPU (Release) - Cross-platform
2. **Microsoft.ML.OnnxRuntime.Gpu** - GPU CUDA (Release) - Windows/Linux/Mac
3. **Microsoft.ML.OnnxRuntime.DirectML** - GPU DirectML (Release) - Windows 10+
4. **onnxruntime** - CPU/GPU Dev builds
5. **Microsoft.ML.OnnxRuntime.Training** - On-Device Training

### C++ API Structure
- C++ API is a thin wrapper around C API
- Provides object-oriented interface
- Memory management through RAII
- Exception-safe operations

### Installation Options
1. **NuGet packages** (Windows)
2. **GitHub releases** (.zip/.tgz files)
3. **Build from source** (full control)
4. **Package managers** (vcpkg, conan)

### Key Classes
- `Ort::Env` - Global environment
- `Ort::Session` - Model inference session
- `Ort::Value` - Input/output tensors
- `Ort::MemoryInfo` - Memory allocation info
- `Ort::SessionOptions` - Configuration options

### Memory Management
- RAII-based resource management
- Automatic cleanup of ONNX resources
- Custom allocators supported
- GPU memory management

### Performance Considerations
- CPU: Intel MKL-DNN, OpenMP
- GPU: CUDA, DirectML, OpenCL
- Mobile: ARM NEON optimizations
- Quantization support (INT8, FP16)

### Model Requirements
- ONNX format (.onnx files)
- Supported operators (check compatibility)
- Input/output tensor specifications
- Batch size considerations

## Implementation Strategy

### For Financial Document Analysis
1. **Text Embeddings**: Use sentence-transformer models
2. **Model Selection**: all-MiniLM-L6-v2 (lightweight, good performance)
3. **Fallback Strategy**: Cloud API if local inference fails
4. **Memory Optimization**: Batch processing, model quantization
5. **Cross-platform**: CPU-only for maximum compatibility

