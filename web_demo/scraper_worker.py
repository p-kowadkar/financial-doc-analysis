"""
Web Scraper using requests + BeautifulSoup (no browser needed).
More reliable than Playwright for systems with content blockers.
"""

import requests
from bs4 import BeautifulSoup
import re
import json


HEADERS = {
    'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36',
    'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8',
    'Accept-Language': 'en-US,en;q=0.5',
}


def search_and_scrape(query: str, max_results: int = 3) -> list:
    """
    Searches DuckDuckGo and scrapes content from top results.
    Uses requests instead of browser for better compatibility.
    """
    results = []
    
    try:
        # Use DuckDuckGo HTML search
        search_url = f"https://html.duckduckgo.com/html/?q={query.replace(' ', '+')}"
        response = requests.get(search_url, headers=HEADERS, timeout=15)
        response.raise_for_status()
        
        soup = BeautifulSoup(response.text, 'html.parser')
        
        # Find result links
        result_divs = soup.select('.result__body')
        
        urls_to_visit = []
        for div in result_divs[:max_results * 2]:
            link = div.select_one('.result__a')
            if link:
                href = link.get('href', '')
                # Extract actual URL from DuckDuckGo redirect
                if '//duckduckgo.com/l/?uddg=' in href:
                    import urllib.parse
                    parsed = urllib.parse.parse_qs(urllib.parse.urlparse(href).query)
                    if 'uddg' in parsed:
                        href = urllib.parse.unquote(parsed['uddg'][0])
                
                if href.startswith('http') and 'duckduckgo.com' not in href:
                    if href not in urls_to_visit:
                        urls_to_visit.append(href)
            
            if len(urls_to_visit) >= max_results:
                break
        
        if not urls_to_visit:
            return [{'title': 'No Results', 'url': '', 'content': f'No search results found for "{query}"'}]
        
        # Scrape each URL
        for url in urls_to_visit:
            try:
                page_resp = requests.get(url, headers=HEADERS, timeout=10)
                page_resp.raise_for_status()
                
                page_soup = BeautifulSoup(page_resp.text, 'html.parser')
                
                # Get title
                title = page_soup.title.string if page_soup.title else "Untitled"
                
                # Remove scripts and styles
                for tag in page_soup(['script', 'style', 'nav', 'header', 'footer', 'aside']):
                    tag.decompose()
                
                # Get main content
                content = ""
                for selector in ['article', 'main', '.content', '#content', '[role="main"]']:
                    element = page_soup.select_one(selector)
                    if element:
                        content = element.get_text(separator=' ', strip=True)
                        if len(content) > 100:
                            break
                
                if not content:
                    content = page_soup.body.get_text(separator=' ', strip=True) if page_soup.body else ""
                
                # Clean and truncate
                content = re.sub(r'\s+', ' ', content).strip()
                content = content[:2000] + "..." if len(content) > 2000 else content
                
                results.append({
                    'title': title[:100] if title else "Untitled",
                    'url': url,
                    'content': content or "Could not extract content."
                })
                
            except Exception as e:
                continue
                
    except Exception as e:
        results.append({'title': 'Search Error', 'url': '', 'content': str(e)})
    
    if not results:
        results.append({'title': 'No Content', 'url': '', 'content': 'Could not extract content from any results.'})
    
    return results


if __name__ == "__main__":
    import sys
    if len(sys.argv) < 2:
        print(json.dumps([{'title': 'Error', 'url': '', 'content': 'No query provided'}]))
    else:
        query = sys.argv[1]
        max_results = int(sys.argv[2]) if len(sys.argv) > 2 else 3
        results = search_and_scrape(query, max_results)
        print(json.dumps(results))
