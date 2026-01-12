from flask import Flask, render_template, request, jsonify
import subprocess
import os
import json
import tempfile
from datetime import datetime
from web_scraper import search_and_scrape

app = Flask(__name__)

# Set the working directory to the src folder
# Determine paths dynamically
BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC_DIR = os.path.join(BASE_DIR, 'src')
OUTPUT_DIR = os.path.join(BASE_DIR, 'output')

# Ensure directories exist
if not os.path.exists(SRC_DIR):
    print(f"Warning: Source directory not found at {SRC_DIR}")
if not os.path.exists(OUTPUT_DIR):
    os.makedirs(OUTPUT_DIR)

@app.route('/')
def index():
    return render_template('index.html')

def validate_input(text):
    """Basic validation to prevent shell injection characters"""
    # Block characters that are dangerous in shell commands
    dangerous_chars = [';', '&', '|', '`', '$', '(', ')', '<', '>', '\\']
    for char in dangerous_chars:
        if char in text:
            return False
    return True

@app.route('/query', methods=['POST'])
def query():
    try:
        data = request.json
        query_text = data.get('query', '')
        top_k = data.get('top_k', 3)
        use_summary = data.get('use_summary', False)
        
        if not query_text:
            return jsonify({'error': 'Query text is required'}), 400
            
        if not validate_input(query_text):
            return jsonify({'error': 'Query contains invalid characters'}), 400
        
        # Generate unique output filename
        timestamp = int(datetime.now().timestamp())
        
        if use_summary:
            output_file = f'{OUTPUT_DIR}/web_rag_result_{timestamp}.md'
            cmd = [f'{SRC_DIR}/rag_engine.exe', f'{OUTPUT_DIR}/financial_docs_index.json', 
                   query_text, output_file, str(top_k)]
        else:
            output_file = f'{OUTPUT_DIR}/web_query_result_{timestamp}.json'
            cmd = [f'{SRC_DIR}/query_engine.exe', f'{OUTPUT_DIR}/financial_docs_index.json', 
                   query_text, output_file, str(top_k), '--json']

        
        # Execute the query
        result = subprocess.run(cmd, capture_output=True, text=True, cwd=SRC_DIR)
        
        if result.returncode != 0:
            return jsonify({'error': f'Query failed: {result.stderr}'}), 500
        
        # Read the result file
        if os.path.exists(output_file):
            with open(output_file, 'r') as f:
                content = f.read()
            
            if use_summary:
                return jsonify({
                    'success': True,
                    'type': 'summary',
                    'content': content,
                    'query': query_text,
                    'top_k': top_k
                })
            else:
                try:
                    json_content = json.loads(content)
                    return jsonify({
                        'success': True,
                        'type': 'results',
                        'content': json_content,
                        'query': query_text,
                        'top_k': top_k
                    })
                except json.JSONDecodeError:
                    return jsonify({
                        'success': True,
                        'type': 'text',
                        'content': content,
                        'query': query_text,
                        'top_k': top_k
                    })
        else:
            return jsonify({'error': 'Output file not found'}), 500
            
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/analyze', methods=['POST'])
def analyze():
    try:
        timestamp = int(datetime.now().timestamp())
        json_output = f'{OUTPUT_DIR}/web_analysis_{timestamp}.json'
        md_output = f'{OUTPUT_DIR}/web_analysis_{timestamp}.md'
        
        cmd = [f'{SRC_DIR}/financial_analyzer.exe', f'{OUTPUT_DIR}/financial_docs_index.json', 
               json_output, md_output]
        
        result = subprocess.run(cmd, capture_output=True, text=True, cwd=SRC_DIR)
        
        if result.returncode != 0:
            return jsonify({'error': f'Analysis failed: {result.stderr}'}), 500
        
        # Read both output files
        analysis_data = {}
        
        if os.path.exists(json_output):
            with open(json_output, 'r') as f:
                analysis_data['json'] = json.loads(f.read())
        
        if os.path.exists(md_output):
            with open(md_output, 'r') as f:
                analysis_data['markdown'] = f.read()
        
        return jsonify({
            'success': True,
            'data': analysis_data
        })
        
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/status')
def status():
    try:
        # Check if index exists
        index_exists = os.path.exists(f'{OUTPUT_DIR}/financial_docs_index.json')
        
        # Get some basic stats
        stats = {}
        if index_exists:
            with open(f'{OUTPUT_DIR}/financial_docs_index.json', 'r') as f:
                index_data = json.loads(f.read())
                stats = {
                    'total_documents': index_data.get('total_documents', 0),
                    'vocabulary_size': index_data.get('vocabulary_size', 0)
                }
        
        return jsonify({
            'system_ready': index_exists,
            'stats': stats,
            'available_commands': ['query', 'analyze']
        })
        
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/scrape', methods=['POST'])
def scrape():
    """Real-time web scraping endpoint"""
    try:
        data = request.json
        query_text = data.get('query', '')
        max_results = data.get('max_results', 3)
        
        if not query_text:
            return jsonify({'error': 'Query text is required'}), 400
        
        # Perform web search and scraping
        results = search_and_scrape(query_text, max_results=max_results)
        
        return jsonify({
            'success': True,
            'type': 'web_results',
            'query': query_text,
            'results': results
        })
        
    except Exception as e:
        return jsonify({'error': str(e)}), 500


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)

