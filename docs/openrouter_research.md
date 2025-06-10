# OpenRouter API Research Notes

## Key Findings

### API Structure
- **Base URL**: `https://openrouter.ai/api/v1/`
- **Authentication**: Bearer token (similar to OpenAI)
- **Compatibility**: OpenAI-compatible API format
- **Unified Interface**: Access to 400+ models through single endpoint

### Available DeepSeek Models (Priority Order)
1. **DeepSeek: R1 0528 (free)** - `deepseek/deepseek-r1-0528`
   - 671B parameters (37B active)
   - 164K context length
   - **FREE** ($0/M input, $0/M output)
   - Performance on par with OpenAI o1
   - Open reasoning tokens

2. **DeepSeek: R1 0528** - `deepseek/deepseek-r1-0528`
   - Same model as above, paid tier
   - 128K context length
   - $0.50/M input, $2.15/M output
   - Higher rate limits

3. **DeepSeek: R1 Distill Qwen 7B** - `deepseek/deepseek-r1-distill-qwen-7b`
   - 7B parameters (distilled from R1)
   - 131K context length
   - $0.10/M input, $0.20/M output
   - Strong math/coding performance

4. **DeepSeek: Deepseek R1 0528 Qwen3 8B (free)** - `deepseek/deepseek-r1-0528-qwen3-8b`
   - 8B parameters
   - 131K context length
   - **FREE** ($0/M input, $0/M output)
   - Chain-of-thought reasoning

### OpenAI Models (Fallback)
- **GPT-4.1** - Latest flagship model
- **GPT-4.1-mini** - Cost-effective option
- **GPT-4o** - Optimized for speed

### API Features
- **Automatic Fallbacks**: If primary model fails, auto-routes to backup
- **Cost Optimization**: Automatically selects most cost-effective option
- **Rate Limiting**: Built-in rate limit handling
- **Streaming**: Real-time response streaming
- **Custom Headers**: Optional app identification for leaderboards

### Request Format
```json
{
  "model": "deepseek/deepseek-r1-0528",
  "messages": [
    {
      "role": "user", 
      "content": "Analyze this financial document..."
    }
  ],
  "max_tokens": 4000,
  "temperature": 0.1,
  "stream": false
}
```

### Response Format
```json
{
  "id": "gen-xxx",
  "object": "chat.completion",
  "created": 1234567890,
  "model": "deepseek/deepseek-r1-0528",
  "choices": [
    {
      "index": 0,
      "message": {
        "role": "assistant",
        "content": "Analysis result..."
      },
      "finish_reason": "stop"
    }
  ],
  "usage": {
    "prompt_tokens": 100,
    "completion_tokens": 500,
    "total_tokens": 600
  }
}
```

## Implementation Strategy

### Model Priority (Cost-Aware)
1. **DeepSeek R1 0528 (free)** - Primary choice (free tier)
2. **DeepSeek R1 Distill Qwen 7B** - Backup (low cost)
3. **GPT-4.1-mini** - Final fallback (OpenAI)

### Use Cases
- **Document Summarization**: DeepSeek R1 (reasoning capabilities)
- **Financial Analysis**: DeepSeek R1 (mathematical reasoning)
- **Query Enhancement**: DeepSeek Distill (cost-effective)
- **Error Recovery**: GPT-4.1-mini (reliable fallback)

### Rate Limiting Strategy
- **Free Models**: Conservative rate limiting
- **Paid Models**: Higher throughput when needed
- **Automatic Backoff**: Exponential retry with fallback models

