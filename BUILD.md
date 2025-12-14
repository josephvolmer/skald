# Building Skald

## Automated Builds (GitHub Actions)

This repository includes GitHub Actions that automatically build VST3 for both Windows and macOS.

### How it works:
1. Push code to GitHub
2. GitHub Actions automatically builds for Windows + macOS
3. Download built plugins from Actions artifacts
4. For releases: Create a tag (e.g., `v1.0.1`) and binaries are auto-released

### Setting up GitHub repository:

```bash
cd TurntableMIDI
git init
git add .
git commit -m "Initial commit - Skald v1.0.0"

# Create repo on GitHub, then:
git remote add origin https://github.com/YOUR_USERNAME/skald.git
git branch -M main
git push -u origin main
```

### To trigger a build:
```bash
# Just push any changes
git add .
git commit -m "Update feature"
git push

# Builds will appear in: Actions tab > Latest workflow > Artifacts
```

### To create a release:
```bash
# Tag a version
git tag v1.0.1
git push origin v1.0.1

# GitHub will automatically:
# - Build Windows + macOS VST3
# - Create a GitHub Release
# - Attach the built plugins as downloadable ZIPs
```

---

## Manual Local Builds

### macOS Build

```bash
# First time setup
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release

# Output:
# VST3: ~/Library/Audio/Plug-Ins/VST3/Skald.vst3
# AU: ~/Library/Audio/Plug-Ins/Components/Skald.component
```

### Windows Build (on Windows PC)

Requirements:
- Visual Studio 2019 or later
- CMake 3.15+

```bash
# Configure
cmake -B build -G "Visual Studio 16 2019"

# Build
cmake --build build --config Release

# Output: build/TurntableMIDI_artefacts/Release/VST3/Skald.vst3
```

### Linux Build

```bash
# Install dependencies
sudo apt-get install build-essential libasound2-dev libx11-dev libxrandr-dev \
  libxinerama-dev libxcursor-dev libfreetype6-dev

# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Output: build/TurntableMIDI_artefacts/Release/VST3/Skald.vst3
```

---

## GitHub Actions Artifacts

After each push, GitHub Actions creates build artifacts:

1. Go to your repo on GitHub
2. Click "Actions" tab
3. Click latest workflow run
4. Scroll down to "Artifacts"
5. Download:
   - `Skald-VST3-Windows` (Windows VST3)
   - `Skald-VST3-macOS` (macOS VST3)

---

## Repository Structure

```
TurntableMIDI/
├── .github/
│   └── workflows/
│       └── build.yml          # Automated build workflow
├── Source/
│   ├── PluginProcessor.cpp
│   ├── PluginProcessor.h
│   ├── PluginEditor.cpp
│   └── PluginEditor.h
├── CMakeLists.txt
├── README.md
├── QUICK_START.md
└── BUILD.md                   # This file
```

Note: JUCE should be in `../JUCE` (parent directory) or update CMakeLists.txt path.

---

## Troubleshooting

### "JUCE not found"
- Ensure JUCE is cloned in parent directory: `../JUCE`
- Or update CMakeLists.txt line 5 to point to your JUCE location

### Windows builds fail on GitHub Actions
- Check that workflow file has correct paths
- Ensure all image assets are committed (viking_full.png, etc.)

### macOS builds fail on GitHub Actions
- Usually works automatically
- Check that JUCE submodule is properly configured

---

## Tips

- **Test locally first** before pushing to GitHub
- **Tag releases** with semantic versioning (v1.0.0, v1.0.1, etc.)
- **Check Actions logs** if builds fail
- **Artifacts expire** after 90 days - download and archive releases

---

Built with ⚔️ by Beowulf Audio
