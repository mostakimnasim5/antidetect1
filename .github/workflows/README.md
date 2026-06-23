# GitHub Actions CI/CD Setup Guide

## Quick Setup (3 Steps)

### Step 1: Create Directory Structure

The workflow files should be in `.github/workflows/` directory:

```
antidetect1/
├── .github/
│   └── workflows/
│       ├── ci.yml          # Main CI pipeline
│       └── pr-checks.yml   # PR-specific checks
├── src/
├── include/
├── CMakeLists.txt
└── ...
```

### Step 2: Files Already Created

✅ `.github/workflows/ci.yml` - Complete CI pipeline
✅ `.github/workflows/pr-checks.yml` - PR checks
✅ `.github/workflows/README.md` - This guide

### Step 3: Push to GitHub

```bash
# Stage the files
git add .github/workflows/

# Commit
git commit -m "Add GitHub Actions CI/CD pipeline

- Multi-platform build (Windows, Linux, macOS)
- Static analysis (clang-tidy, cppcheck)
- Code quality checks
- Security scanning
- PR-specific checks
- Automatic deployment"

# Push
git push origin main
```

---

## What Each Workflow Does

### 1. `ci.yml` - Main CI Pipeline

**Triggers on:**
- Every push to any branch
- Pull requests to main/develop
- Manual workflow dispatch

**Jobs:**

| Job | Platform | Compiler | Purpose |
|-----|----------|---------|---------|
| `analyze` | Ubuntu | - | Static analysis |
| `build-linux` | Ubuntu 22.04 | GCC 13 | Linux build |
| `build-linux-clang` | Ubuntu 22.04 | Clang 17 | Linux (Clang) |
| `build-windows-msvc` | Windows 2022 | MSVC 17.8 | Windows (MSVC) |
| `build-windows-mingw` | Windows 2022 | MinGW | Windows (MinGW) |
| `build-macos` | macOS 14 | Clang + Qt6 | macOS build |
| `code-quality` | Ubuntu | - | Statistics |
| `security-scan` | Ubuntu | - | Security |
| `deploy` | Ubuntu | - | Release |

### 2. `pr-checks.yml` - PR Checks

**Triggers on:**
- PR opened
- PR updated (new commits)
- PR reopened

**Jobs:**

| Job | Purpose |
|-----|---------|
| `quick-checks` | File changes, large files |
| `compile-check` | Fast compilation test |
| `lint-check` | Code style checks |
| `pr-size-check` | PR size analysis |

---

## GitHub Repository Setup

### Method 1: Direct Push (Recommended)

```bash
cd antidetect1
git add .github/workflows/
git commit -m "Add GitHub Actions CI/CD"
git push origin main
```

### Method 2: Create PR

```bash
# Create new branch
git checkout -b feature/github-actions-ci

# Add and commit
git add .github/workflows/
git commit -m "Add GitHub Actions CI/CD"

# Push and create PR
git push -u origin feature/github-actions-ci
```

Then create PR via GitHub UI.

---

## Verifying the Setup

### 1. Check Workflows in GitHub

1. Go to: `https://github.com/mostakimnasim5/antidetect1/actions`
2. You should see the new workflows listed

### 2. Trigger a Test Run

```bash
# Push a small change to trigger CI
echo "# Test" >> README.md
git add README.md
git commit -m "Test CI trigger"
git push origin main
```

### 3. View Results

1. Go to Actions tab
2. Click on the workflow run
3. See all jobs and their status

---

## Customization

### Change Qt Version

Edit `.github/workflows/ci.yml`:

```yaml
env:
  QT_VERSION: '6.6.1'  # Change to your version
```

### Add More Platforms

Add new jobs in the `jobs:` section:

```yaml
build-freebsd:
  name: Build FreeBSD
  runs-on: ubuntu-latest
  container: freebsd:latest
  steps:
    - uses: actions/checkout@v4
    # ... your steps
```

### Disable Some Jobs

Comment out or delete jobs you don't need.

### Add Secrets

For private dependencies:

1. Go to Settings → Secrets and variables → Actions
2. Add new repository secret
3. Use in workflow: `${{ secrets.SECRET_NAME }}`

---

## Troubleshooting

### Build Fails on Windows

```bash
# Check CMakeLists.txt compatibility
# Ensure you have:
cmake_minimum_required(VERSION 3.20)
```

### Qt6 Not Found

```bash
# Qt version in workflow
uses: jurplel/qt-action@v3
with:
  version: '6.6.1'
```

### Permission Issues

Add to workflow:

```yaml
permissions:
  contents: read
  checks: write
  pull-requests: write
```

---

## Expected Runtime

| Job | Estimated Time |
|-----|---------------|
| analyze | ~2 min |
| build-linux | ~3 min |
| build-linux-clang | ~3 min |
| build-windows-msvc | ~5 min |
| build-windows-mingw | ~4 min |
| build-macos | ~6 min |
| code-quality | ~1 min |
| security-scan | ~2 min |
| **Total** | ~15-20 min |

---

## Artifacts

Build artifacts are automatically uploaded:

- `linux-build-*.tar.gz` - Linux binaries
- `windows-build-msvc-*.zip` - Windows MSVC binaries
- `macos-build-*.tar.gz` - macOS binaries

Download from Actions → Artifacts tab.

---

## Questions?

Check GitHub Actions documentation:
https://docs.github.com/en/actions
