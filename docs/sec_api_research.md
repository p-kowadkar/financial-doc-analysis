# SEC EDGAR API Research Notes

## Key API Endpoints

### 1. Submissions API
- **URL Pattern**: `https://data.sec.gov/submissions/CIK##########.json`
- **Purpose**: Get filing history for a specific company
- **Data**: Metadata, filing history, ticker symbols
- **Rate Limit**: 10 requests per second
- **Authentication**: None required

### 2. Company Facts API  
- **URL Pattern**: `https://data.sec.gov/api/xbrl/companyfacts/CIK##########.json`
- **Purpose**: All XBRL financial data for a company
- **Data**: Financial statements, metrics, concepts

### 3. Company Concept API
- **URL Pattern**: `https://data.sec.gov/api/xbrl/companyconcept/CIK##########/us-gaap/AccountsPayableCurrent.json`
- **Purpose**: Specific financial concept across time
- **Data**: Single metric historical data

### 4. Frames API
- **URL Pattern**: `https://data.sec.gov/api/xbrl/frames/us-gaap/AccountsPayableCurrent/USD/CY2019Q1I.json`
- **Purpose**: Industry-wide data for specific periods
- **Data**: Cross-company comparisons

## Important Notes
- No CORS support (server-side only)
- Real-time updates (< 1 second for submissions, < 1 minute for XBRL)
- Bulk downloads available nightly at 3 AM ET
- Must comply with SEC Privacy and Security Policy
- CIK format: 10-digit with leading zeros

## Filing Types Available
- 10-K, 10-Q, 8-K (US companies)
- 20-F, 40-F, 6-K (foreign companies)
- All variants and amendments

## Rate Limiting
- 10 requests per second maximum
- Must include User-Agent header with contact info
- Recommended to use bulk downloads for large datasets

