# Setting Up GitHub for Automated Builds

This guide will help you set up GitHub to automatically build Skald for Windows and macOS.

## Step 1: Create GitHub Repository

1. Go to https://github.com/new
2. Repository name: `skald` (or whatever you prefer)
3. Description: `Skald - Viking MIDI Warrior | Generative MIDI Sequencer`
4. Choose: **Public** (for free GitHub Actions) or **Private**
5. **Do NOT** initialize with README (we already have one)
6. Click **Create repository**

## Step 2: Prepare JUCE

GitHub Actions needs JUCE to build. You have two options:

### Option A: JUCE as Git Submodule (Recommended)

```bash
cd /Users/beowulf/Desktop/claude-vsts/Skald

# Add JUCE as submodule
git submodule add https://github.com/juce-framework/JUCE.git ../JUCE
git submodule update --init --recursive
```

Then update CMakeLists.txt line 5:
```cmake
add_subdirectory(JUCE ${CMAKE_CURRENT_BINARY_DIR}/JUCE)
```

### Option B: Copy JUCE into repo (Simpler but larger)

```bash
cd /Users/beowulf/Desktop/claude-vsts/Skald
mkdir JUCE
cp -r ../JUCE/* JUCE/
```

Then update CMakeLists.txt line 5:
```cmake
add_subdirectory(JUCE ${CMAKE_CURRENT_BINARY_DIR}/JUCE)
```

## Step 3: Initialize Git Repository

```bash
cd /Users/beowulf/Desktop/claude-vsts/Skald

# Initialize git
git init

# Add all files
git add .

# First commit
git commit -m "Initial commit - Skald v1.0.0

- VST3 + AU for macOS
- 13 musical scales
- Probability, swing, velocity variation
- Motor control and scratching
- Save/Load patterns
- GitHub Actions for cross-platform builds"
```

## Step 4: Connect to GitHub

```bash
# Replace YOUR_USERNAME with your GitHub username
git remote add origin https://github.com/YOUR_USERNAME/skald.git

# Rename branch to main (if needed)
git branch -M main

# Push to GitHub
git push -u origin main
```

## Step 5: Verify GitHub Actions

1. Go to your repository on GitHub
2. Click the **"Actions"** tab
3. You should see a workflow running automatically!
4. Wait for it to complete (5-10 minutes)

## Step 6: Download Built Plugins

Once the workflow completes:

1. Click on the completed workflow run
2. Scroll down to **"Artifacts"**
3. Download:
   - **Skald-VST3-Windows.zip** - Windows VST3
   - **Skald-VST3-macOS.zip** - macOS VST3

## Step 7: Create a Release (Optional)

To create downloadable releases:

```bash
# Tag the current version
git tag v1.0.0

# Push the tag
git push origin v1.0.0
```

This will:
- Trigger GitHub Actions
- Build for Windows + macOS
- Create a GitHub Release automatically
- Attach built plugins as downloadable files

## Step 8: Update Your Gumroad Package

Now you have both Windows and macOS builds!

```bash
# Download artifacts from GitHub Actions
# Then update your package:

cd ~/Desktop
mkdir Skald-v1.0.0-Complete

# macOS
mkdir Skald-v1.0.0-Complete/macOS
cp ~/Downloads/Skald-VST3-macOS/* Skald-v1.0.0-Complete/macOS/

# Windows
mkdir Skald-v1.0.0-Complete/Windows
cp ~/Downloads/Skald-VST3-Windows/* Skald-v1.0.0-Complete/Windows/

# Documentation
mkdir Skald-v1.0.0-Complete/Documentation
cp /Users/beowulf/Desktop/claude-vsts/Skald/README.md Skald-v1.0.0-Complete/Documentation/
cp /Users/beowulf/Desktop/claude-vsts/Skald/QUICK_START.md Skald-v1.0.0-Complete/Documentation/

# Create README and installation guide
# (Use the files you already created)

# Zip it
zip -r Skald-v1.0.0-Complete.zip Skald-v1.0.0-Complete
```

## Troubleshooting

### "JUCE not found" error in GitHub Actions

Update `.github/workflows/build.yml` to checkout JUCE:

```yaml
- name: Checkout code
  uses: actions/checkout@v3
  with:
    submodules: recursive  # This line is important!
```

### Builds are slow

- First build takes 5-10 minutes (JUCE compilation)
- Subsequent builds are faster (~3-5 minutes)
- Use caching (already configured in workflow)

### Need to build locally while developing?

```bash
# macOS
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Plugins auto-install to:
# ~/Library/Audio/Plug-Ins/VST3/Skald.vst3
```

---

## Quick Reference

### Push code changes
```bash
git add .
git commit -m "Add new feature"
git push
```

### Create new release
```bash
git tag v1.0.1
git push origin v1.0.1
# Download from GitHub Releases page
```

### Check build status
Go to: https://github.com/YOUR_USERNAME/skald/actions

---

## What You Get

✅ Automatic builds for Windows + macOS
✅ No need for Windows PC
✅ No manual compilation
✅ Professional release workflow
✅ Free (if public repo)

---

**Ready to ship cross-platform!** 🚀⚔️

Built with ⚔️ by Beowulf Audio
