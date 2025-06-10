# Step-by-Step Branch Protection Setup

Based on your current GitHub interface, here's exactly what to configure:

## Current Settings You Should Configure:

### 1. Ruleset Name
- Change "Test rules" to: `main-branch-protection`

### 2. Enforcement Status
- Keep it as "Disabled" for now (we'll enable after testing)

### 3. Target Branches
- Click "Add target" 
- In the "Branch targeting criteria" field, enter: `main`
- Click the checkmark to save

### 4. Branch Rules (Check these boxes):

#### ✅ **Require a pull request before merging** (MUST CHECK)
- This is the most important rule
- After checking this, click "Show additional settings"
- Set "Required number of approvals before merging" to: `1`
- Check "Dismiss stale PR approvals when new commits are pushed"

#### ✅ **Require status checks to pass** (MUST CHECK) 
- This ensures CI tests pass before merging
- After checking this, click "Show additional settings"
- Check "Require branches to be up to date before merging"
- In the status checks field, you'll add: `test` and `build-windows` (but only after the CI runs once)

#### ✅ **Block force pushes** (ALREADY CHECKED - GOOD)
- Keep this checked for safety

#### ✅ **Restrict deletions** (ALREADY CHECKED - GOOD)  
- Keep this checked for safety

#### Optional but Recommended:
- ✅ **Require linear history** - keeps git history clean
- ❌ **Require deployments to succeed** - leave unchecked for now
- ❌ **Require signed commits** - leave unchecked unless you use GPG
- ❌ **Require code scanning results** - leave unchecked for now

### 5. Click "Create" at the bottom

## After Creating the Rule:

### Step 1: Test the CI Pipeline First
1. Go back to your terminal/VS Code
2. Make a small change to trigger CI
3. Push to master branch
4. Wait for CI to run successfully

### Step 2: Add Status Checks
1. Come back to this rule after CI runs
2. Edit the rule
3. Under "Require status checks to pass", add:
   - `test`
   - `build-windows`

### Step 3: Enable the Rule
1. Change "Enforcement status" from "Disabled" to "Active"
2. Save the changes

## What This Will Do:
- ✅ Prevent direct pushes to main branch
- ✅ Require pull requests for all changes
- ✅ Require CI tests to pass before merging
- ✅ Prevent force pushes and branch deletion
- ✅ Require code review approval

## Next Steps After Setup:
1. Create a PR from master to main
2. Verify CI runs automatically
3. Merge the PR to test the full workflow
