# Branch Protection Rules Setup Guide

## Setting up Branch Protection for Main Branch

### Step 1: Access Branch Protection Settings
1. Go to your repository on GitHub
2. Click the "Settings" tab
3. Click "Branches" in the left sidebar
4. Click "Add rule" under "Branch protection rules"

### Step 2: Configure the Rule
1. **Branch name pattern**: Enter `main`
2. **Protection settings to enable**:
   - ✅ **Require a pull request before merging**
     - ✅ Require approvals (set to 1)
     - ✅ Dismiss stale PR approvals when new commits are pushed
   - ✅ **Require status checks to pass before merging**
     - ✅ Require branches to be up to date before merging
     - In the search box, add these status checks:
       - `test`
       - `build-windows`
   - ✅ **Require conversation resolution before merging**
   - ✅ **Restrict pushes that create files larger than 100MB**

### Step 3: Additional Settings (Optional)
- **Require linear history**: ✅ (keeps git history clean)
- **Allow force pushes**: ❌ (keep disabled for safety)
- **Allow deletions**: ❌ (keep disabled for safety)

### Step 4: Save the Rule
Click "Create" to save the branch protection rule.

## Testing the Setup

### Create a Pull Request
1. Make sure you're on the master branch locally
2. Create a small test change
3. Push to master
4. Go to GitHub and create a PR from master to main
5. Verify that CI checks run automatically
6. Verify that you cannot merge until checks pass

### Expected Behavior
- ✅ CI/CD pipeline runs on PR creation
- ✅ Both `test` and `build-windows` jobs must pass
- ✅ Cannot merge without approval (if you have collaborators)
- ✅ Cannot merge until status checks pass

## Troubleshooting

### If "Include administrators" option is missing:
This option may not be available for:
- Personal repositories (only organization repos)
- Free GitHub accounts
- Repositories with only one collaborator

This is normal and doesn't affect the protection functionality.

### If status checks don't appear:
1. Make sure the CI workflow has run at least once
2. The status check names must match exactly what's in your workflow file
3. Wait a few minutes after the first workflow run

### Common Issues:
1. **Status checks not found**: Run the workflow at least once first
2. **Cannot create PR**: Make sure branches are different
3. **Checks not running**: Verify workflow triggers in `.github/workflows/ci.yml`

## Next Steps After Setup
1. Test by creating a PR from master to main
2. Verify CI runs automatically
3. Merge the PR once checks pass
4. Confirm deployment pipeline runs on main branch
