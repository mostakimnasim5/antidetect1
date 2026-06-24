# GitHub Actions CI/CD Setup Guide

## ✅ Setup Complete!

Your CI workflow is configured at: `.github/workflows/ci.yml`

---

## 🖥️ Build Platforms

| Platform | Compiler | Purpose |
|----------|----------|---------|
| Ubuntu 22.04 | GCC-11 | Linux production build + static analysis |
| Windows 2022 | MSVC 14.37 | Windows Visual Studio build |
| Windows (MinGW) | GCC-13 | Windows portable build |
| macOS 14 | Apple Clang | macOS build |
| Code Quality | - | cpplint, cmake-format, header guards |

---

## 🚀 How It Works

### Automatic Triggers
```
✅ Push to main/develop → CI runs
✅ Pull Request → CI runs  
✅ Version tag (v*) → CI runs
✅ Manual (workflow_dispatch) → CI runs
```

---

## 📋 What Gets Tested

1. **Compilation** - No errors or warnings
2. **Static Analysis** - Cppcheck memory/UB issues
3. **Code Quality** - cpplint, formatting, header guards
4. **Multi-platform** - Linux, Windows (MSVC + MinGW), macOS

---

## 🔍 View Results

1. Go to: https://github.com/mostakimnasim5/antidetect1/actions
2. Click on any workflow run
3. Check build status per platform
4. Download logs from **Artifacts** on failure

---

## ⏱️ Expected Runtime

| Job | Time |
|-----|------|
| Ubuntu GCC | ~3-5 min |
| Windows MSVC | ~4-6 min |
| Windows MinGW | ~3-5 min |
| macOS | ~4-6 min |
| Code Quality | ~1-2 min |
| **Total** | ~15-20 min |

---

## 📦 Artifacts (on failure)

- `ubuntu-build-logs` - Linux compilation logs
- `windows-build-logs` - MSVC logs
- `mingw-build-logs` - MinGW logs
- `cppcheck_report.txt` - Static analysis report

---

## 🔧 Customize

### Skip CI for a commit
```bash
git commit -m "Fix typo [skip ci]"
```

### Add CI badge to README
```markdown
[![C++ CI](https://github.com/mostakimnasim5/antidetect1/actions/workflows/ci.yml/badge.svg)](https://github.com/mostakimnasim5/antidetect1/actions/workflows/ci.yml)
```

---

## ❓ Common Issues

| Issue | Solution |
|-------|----------|
| OpenSSL not found | Check workflow installs libssl-dev |
| MSVC fails | Update msvc_version to 14.37+ |
| Timeout | Increase timeout in ctest step |
