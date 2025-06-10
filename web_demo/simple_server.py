#!/usr/bin/env python3
import http.server
import socketserver
import json
import subprocess
import os
import tempfile
from urllib.parse import parse_qs, urlparse
from datetime import datetime
import threading
import time

# Set working directories
SRC_DIR = '/home/ubuntu/financial-doc-analysis-windows/src'
OUTPUT_DIR = '/home/ubuntu/financial-doc-analysis-windows/output'

class FinancialAnalysisHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/' or self.path == '/index.html':
            self.serve_index()
        elif self.path == '/status':
            self.serve_status()
        else:
            super().do_GET()
    
    def do_POST(self):
        if self.path == '/query':
            self.handle_query()
        elif self.path == '/analyze':
            self.handle_analyze()
        else:
            self.send_error(404)
    
    def serve_index(self):
        html_content = '''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>C++ Financial Document Analysis System - Live Demo</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }
        .container { max-width: 1200px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }
        .header { text-align: center; margin-bottom: 30px; padding: 20px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; border-radius: 10px; }
        .section { margin-bottom: 30px; padding: 20px; border: 1px solid #ddd; border-radius: 8px; }
        .btn { background: #667eea; color: white; border: none; padding: 12px 24px; border-radius: 6px; cursor: pointer; font-size: 16px; }
        .btn:hover { background: #5a6fd8; }
        .btn-secondary { background: #28a745; }
        .btn-secondary:hover { background: #218838; }
        input, select { padding: 10px; margin: 5px; border: 1px solid #ddd; border-radius: 4px; font-size: 16px; }
        .query-input { width: 400px; }
        .results { margin-top: 20px; padding: 20px; background: #f8f9fa; border-radius: 8px; }
        .result-item { background: white; margin: 10px 0; padding: 15px; border-radius: 6px; border-left: 4px solid #667eea; }
        .loading { text-align: center; padding: 20px; color: #666; }
        .error { background: #f8d7da; color: #721c24; padding: 15px; border-radius: 6px; }
        .sample-queries { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 15px; margin: 20px 0; }
        .sample-query { background: #e9ecef; padding: 15px; border-radius: 6px; cursor: pointer; transition: background 0.3s; }
        .sample-query:hover { background: #dee2e6; }
        .status-bar { background: #d4edda; padding: 10px; border-radius: 6px; margin-bottom: 20px; text-align: center; }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🔍 C++ Financial Document Analysis System</h1>
            <p>Live Demo - Zero Dependencies, Pure C++ Implementation</p>
        </div>
        
        <div class="status-bar">
            <strong>✅ System Status:</strong> <span id="status">Ready</span> | 
            <span id="stats">Documents: 14, Vocabulary: 531</span>
        </div>
        
        <div class="section">
            <h2>📊 Document Query & Analysis</h2>
            <p>Query the financial documents using natural language. The system uses TF-IDF similarity search.</p>
            
            <div style="margin: 20px 0;">
                <input type="text" id="query-input" class="query-input" placeholder="Enter your query (e.g., cybersecurity risks)">
                <select id="top-k">
                    <option value="3">Top 3</option>
                    <option value="5">Top 5</option>
                    <option value="10">Top 10</option>
                </select>
                <label><input type="checkbox" id="use-summary"> Generate Summary</label>
                <button class="btn" onclick="executeQuery()">🔍 Search</button>
            </div>
            
            <div class="sample-queries">
                <div class="sample-query" onclick="setQuery('cybersecurity risks and data protection')">
                    <strong>🔒 Cybersecurity Analysis</strong><br>
                    Analyze cybersecurity risks and data protection measures
                </div>
                <div class="sample-query" onclick="setQuery('revenue growth and financial performance')">
                    <strong>📈 Financial Performance</strong><br>
                    Examine revenue trends and financial metrics
                </div>
                <div class="sample-query" onclick="setQuery('competition and market risks')">
                    <strong>🏢 Market Competition</strong><br>
                    Review competitive landscape and market challenges
                </div>
                <div class="sample-query" onclick="setQuery('regulatory compliance and legal proceedings')">
                    <strong>⚖️ Regulatory Compliance</strong><br>
                    Investigate regulatory requirements and legal issues
                </div>
            </div>
            
            <div id="query-results" class="results" style="display: none;"></div>
        </div>
        
        <div class="section">
            <h2>📋 Financial Analysis</h2>
            <p>Perform comprehensive financial analysis including risk assessment and metric extraction.</p>
            
            <button class="btn btn-secondary" onclick="performAnalysis()">📊 Run Financial Analysis</button>
            
            <div id="analysis-results" class="results" style="display: none;"></div>
        </div>
    </div>

    <script>
        function setQuery(query) {
            document.getElementById('query-input').value = query;
        }
        
        function executeQuery() {
            const query = document.getElementById('query-input').value.trim();
            const topK = parseInt(document.getElementById('top-k').value);
            const useSummary = document.getElementById('use-summary').checked;
            
            if (!query) {
                alert('Please enter a query');
                return;
            }
            
            const resultsDiv = document.getElementById('query-results');
            resultsDiv.style.display = 'block';
            resultsDiv.innerHTML = '<div class="loading">🔄 Processing query: "' + query + '"...</div>';
            
            fetch('/query', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ query: query, top_k: topK, use_summary: useSummary })
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    displayQueryResults(data);
                } else {
                    resultsDiv.innerHTML = '<div class="error">Error: ' + data.error + '</div>';
                }
            })
            .catch(error => {
                resultsDiv.innerHTML = '<div class="error">Network error: ' + error.message + '</div>';
            });
        }
        
        function displayQueryResults(data) {
            const resultsDiv = document.getElementById('query-results');
            let html = '<h3>🔍 Results for: "' + data.query + '"</h3>';
            
            if (data.type === 'summary') {
                html += '<div class="result-item"><pre style="white-space: pre-wrap;">' + data.content + '</pre></div>';
            } else if (data.content && data.content.results) {
                data.content.results.forEach((result, index) => {
                    html += '<div class="result-item">';
                    html += '<strong>' + (index + 1) + '. ' + result.document_id + '</strong> ';
                    html += '<span style="background: #667eea; color: white; padding: 2px 8px; border-radius: 12px; font-size: 0.9em;">Score: ' + result.similarity_score + '</span>';
                    html += '<div style="margin-top: 10px;">' + result.content_preview + '</div>';
                    html += '</div>';
                });
            } else {
                html += '<div class="result-item"><pre style="white-space: pre-wrap;">' + JSON.stringify(data.content, null, 2) + '</pre></div>';
            }
            
            resultsDiv.innerHTML = html;
        }
        
        function performAnalysis() {
            const resultsDiv = document.getElementById('analysis-results');
            resultsDiv.style.display = 'block';
            resultsDiv.innerHTML = '<div class="loading">🔄 Performing financial analysis...</div>';
            
            fetch('/analyze', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' }
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    displayAnalysisResults(data.data);
                } else {
                    resultsDiv.innerHTML = '<div class="error">Error: ' + data.error + '</div>';
                }
            })
            .catch(error => {
                resultsDiv.innerHTML = '<div class="error">Network error: ' + error.message + '</div>';
            });
        }
        
        function displayAnalysisResults(data) {
            const resultsDiv = document.getElementById('analysis-results');
            let html = '<h3>📊 Financial Analysis Results</h3>';
            
            if (data.json && data.json.companies) {
                data.json.companies.forEach(company => {
                    html += '<div class="result-item">';
                    html += '<h4>' + company.name + '</h4>';
                    html += '<p><strong>Industry:</strong> ' + company.industry + '</p>';
                    
                    if (company.metrics && company.metrics.length > 0) {
                        html += '<h5>Key Metrics:</h5>';
                        company.metrics.forEach(metric => {
                            html += '<div>' + metric.name + ': $' + (metric.value / 1000000000).toFixed(2) + 'B</div>';
                        });
                    }
                    
                    if (company.risks && company.risks.length > 0) {
                        html += '<h5>Risk Factors:</h5>';
                        company.risks.forEach(risk => {
                            const stars = '★'.repeat(Math.round(risk.severity * 5)) + '☆'.repeat(5 - Math.round(risk.severity * 5));
                            html += '<div><strong>' + risk.category + ':</strong> ' + stars + ' (' + risk.mention_count + ' mentions)</div>';
                        });
                    }
                    
                    html += '</div>';
                });
            }
            
            if (data.markdown) {
                html += '<div class="result-item"><h4>📄 Detailed Report</h4>';
                html += '<pre style="white-space: pre-wrap;">' + data.markdown + '</pre></div>';
            }
            
            resultsDiv.innerHTML = html;
        }
    </script>
</body>
</html>'''
        
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        self.wfile.write(html_content.encode())
    
    def serve_status(self):
        try:
            index_exists = os.path.exists(f'{OUTPUT_DIR}/financial_docs_index.json')
            stats = {}
            if index_exists:
                with open(f'{OUTPUT_DIR}/financial_docs_index.json', 'r') as f:
                    index_data = json.loads(f.read())
                    stats = {
                        'total_documents': index_data.get('total_documents', 14),
                        'vocabulary_size': index_data.get('vocabulary_size', 531)
                    }
            
            response = {
                'system_ready': index_exists,
                'stats': stats
            }
        except Exception as e:
            response = {'error': str(e)}
        
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(json.dumps(response).encode())
    
    def handle_query(self):
        try:
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            data = json.loads(post_data.decode())
            
            query_text = data.get('query', '')
            top_k = data.get('top_k', 3)
            use_summary = data.get('use_summary', False)
            
            if not query_text:
                self.send_json_response({'error': 'Query text is required'}, 400)
                return
            
            timestamp = int(datetime.now().timestamp())
            
            if use_summary:
                output_file = f'{OUTPUT_DIR}/web_rag_result_{timestamp}.md'
                cmd = [f'{SRC_DIR}/rag_engine', f'{OUTPUT_DIR}/financial_docs_index.json', 
                       query_text, output_file, str(top_k)]
            else:
                output_file = f'{OUTPUT_DIR}/web_query_result_{timestamp}.json'
                cmd = [f'{SRC_DIR}/query_engine', f'{OUTPUT_DIR}/financial_docs_index.json', 
                       query_text, output_file, str(top_k), '--json']
            
            result = subprocess.run(cmd, capture_output=True, text=True, cwd=SRC_DIR)
            
            if result.returncode != 0:
                self.send_json_response({'error': f'Query failed: {result.stderr}'}, 500)
                return
            
            if os.path.exists(output_file):
                with open(output_file, 'r') as f:
                    content = f.read()
                
                if use_summary:
                    response = {
                        'success': True,
                        'type': 'summary',
                        'content': content,
                        'query': query_text,
                        'top_k': top_k
                    }
                else:
                    try:
                        json_content = json.loads(content)
                        response = {
                            'success': True,
                            'type': 'results',
                            'content': json_content,
                            'query': query_text,
                            'top_k': top_k
                        }
                    except json.JSONDecodeError:
                        response = {
                            'success': True,
                            'type': 'text',
                            'content': content,
                            'query': query_text,
                            'top_k': top_k
                        }
                
                self.send_json_response(response)
            else:
                self.send_json_response({'error': 'Output file not found'}, 500)
                
        except Exception as e:
            self.send_json_response({'error': str(e)}, 500)
    
    def handle_analyze(self):
        try:
            timestamp = int(datetime.now().timestamp())
            json_output = f'{OUTPUT_DIR}/web_analysis_{timestamp}.json'
            md_output = f'{OUTPUT_DIR}/web_analysis_{timestamp}.md'
            
            cmd = [f'{SRC_DIR}/financial_analyzer', f'{OUTPUT_DIR}/financial_docs_index.json', 
                   json_output, md_output]
            
            result = subprocess.run(cmd, capture_output=True, text=True, cwd=SRC_DIR)
            
            if result.returncode != 0:
                self.send_json_response({'error': f'Analysis failed: {result.stderr}'}, 500)
                return
            
            analysis_data = {}
            
            if os.path.exists(json_output):
                with open(json_output, 'r') as f:
                    analysis_data['json'] = json.loads(f.read())
            
            if os.path.exists(md_output):
                with open(md_output, 'r') as f:
                    analysis_data['markdown'] = f.read()
            
            self.send_json_response({'success': True, 'data': analysis_data})
            
        except Exception as e:
            self.send_json_response({'error': str(e)}, 500)
    
    def send_json_response(self, data, status=200):
        self.send_response(status)
        self.send_header('Content-type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()
        self.wfile.write(json.dumps(data).encode())

if __name__ == "__main__":
    PORT = 5000
    os.chdir('/home/ubuntu/financial-doc-analysis-windows/web_demo')
    
    with socketserver.TCPServer(("", PORT), FinancialAnalysisHandler) as httpd:
        print(f"Server running on port {PORT}")
        httpd.serve_forever()

