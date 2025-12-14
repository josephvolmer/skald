# Skald - Viking MIDI Warrior

**A generative MIDI sequencer inspired by mechanical rhythm machines**

![Version](https://img.shields.io/badge/version-1.0.0-orange)
![License](https://img.shields.io/badge/license-GPL--3.0-blue)
![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Windows%20%7C%20Linux-lightgrey)

---

## Overview

**Skald** is a generative MIDI sequencer inspired by Quintron's Drum Buddy and Playtonica MIDI Color Sequencer Orbita - mechanical rhythm machines that merge analog charm with hands-on performance.

Place notes on concentric rings, scratch like vinyl, and explore generative patterns with motor control, probability, swing, and velocity variation.

### Key Features

- 🎯 **Intuitive Turntable Interface** - Visual feedback with rotating sensor arm
- 🎵 **Musical Scales** - Major, Minor, Pentatonic, Blues, Dorian, and more
- 🎲 **Generative Controls** - Probability, velocity variation, and swing
- 🎛️ **Performance Ready** - Scratching, motor control, reverse playback
- 💾 **Save/Load Patterns** - Build your own pattern library
- 🎨 **Multiple Rings** - 12 concentric rings for melodic sequencing
- ⚡ **BPM Sync** - Locks to your DAW's tempo

---

## Installation

### Automatic Installation (macOS)

When you build Skald, it's automatically installed to:

- **VST3**: `~/Library/Audio/Plug-Ins/VST3/Skald.vst3`
- **AU**: `~/Library/Audio/Plug-Ins/Components/Skald.component`
- **Standalone**: `build/TurntableMIDI_artefacts/Release/Standalone/Skald.app`

### Building from Source

```bash
# Clone or navigate to project
cd TurntableMIDI

# Configure CMake (first time only)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build and install
cmake --build build --config Release
```

The plugin will automatically install to your system plugin folders.

---

## Quick Start

### Setup in Your DAW

**IMPORTANT**: Skald is a MIDI generator, not an instrument!

1. Insert Skald on its own MIDI track (leave empty, no instruments)
2. Create a separate MIDI track with your synth/instrument
3. Route MIDI from Skald's track to your synth track
4. Add dots and generate patterns!

See [QUICK_START.md](QUICK_START.md) for detailed DAW-specific instructions.

---

## Features

### Turntable Controls

- **Double-click** to add/remove dots
- **Drag dots** to change timing and pitch
- **Click outer ring** to scratch (vinyl-style control)
- **Motor toggle** for motorized vs manual playback
- **Reverse** for backward playback

### Musical Parameters

- **SCALE**: 13 musical scales (Major, Minor, Pentatonic, Blues, etc.)
- **KEY**: All 12 chromatic root notes
- **OCTAVE**: ±2 octave range
- **SPEED/DIV**: 0.25x to 4x tempo divisions

### Expression Controls

- **PROBABILITY (0-100%)**: Random note triggering
- **SWING (0-100%)**: Groove timing (50% = straight, 66% = triplet)
- **VELOCITY (1-127)**: Global note dynamics
- **VEL VARIATION (0-100%)**: Humanize with random velocity
- **GATE TIME**: Note length in milliseconds

### Pattern Management

- **RANDOMIZE**: Generate instant creative patterns
- **SAVE**: Store your favorite patterns
- **LOAD**: Recall saved patterns
- **CLEAR**: Remove all dots

---

## Interface

```
┌─────────────────────────────────────────────────────────────┐
│  SCALE  KEY   DIV  OCT     [PROB] [SWING] [VEL] [VVAR] [GATE]│
│  Penta   C    1x   0                                         │
│                                                               │
│                         ╱│╲                                  │
│                        ╱ │ ╲                                 │
│                       ╱  │  ╲                                │
│                      ╱ ● │   ╲    ← Dots on rings           │
│                     │    │  ●  │                             │
│                     │  ● │     │                             │
│                      ╲   │  ● ╱                              │
│                       ╲  │  ╱                                │
│                        ╲ │ ╱                                 │
│                         ╲│╱                                  │
│                                                               │
│  [REV] [MOTOR]        [ADD] [RAND] [CLR] [SAVE] [LOAD] [HELP]│
└─────────────────────────────────────────────────────────────┘
```

---

## Technical Specifications

### Plugin Formats
- **VST3** (all platforms)
- **Audio Unit** (macOS)
- **Standalone** (all platforms)

### Requirements
- **JUCE Framework** 7.x or later
- **CMake** 3.15 or later
- **C++17** compatible compiler
- **macOS** 10.13+, **Windows** 10+, or **Linux** (Ubuntu 20.04+)

### Audio/MIDI
- **MIDI Output**: Generates MIDI notes (no audio processing)
- **Channels**: Supports MIDI channels 1-16 (currently channel 1)
- **BPM Sync**: Automatically syncs to host DAW tempo
- **Note Range**: Configurable via scales and octave shift

---

## Project Structure

```
TurntableMIDI/
├── CMakeLists.txt              # Build configuration
├── Source/
│   ├── PluginProcessor.h       # MIDI generation engine
│   ├── PluginProcessor.cpp     # Pattern playback & timing
│   ├── PluginEditor.h          # GUI interface
│   └── PluginEditor.cpp        # Turntable visualization
├── viking_full.png             # Viking warrior graphic
├── viking_head.png             # Viking head icon
├── switch_toggle.png           # Toggle switch sprite
├── knob_simplegray.png         # Knob sprite
├── wallpaper.jpg               # Background texture
├── README.md                   # This file
└── QUICK_START.md              # Quick start guide
```

---

## Development

### Modifying the Code

1. Edit files in `Source/`
2. Rebuild: `cmake --build build --config Release`
3. Plugin automatically reinstalls

### Key Files

- **PluginProcessor.cpp**: Pattern logic, MIDI generation, timing
- **PluginEditor.cpp**: UI, graphics, user interaction
- **PluginProcessor.h**: Scale definitions, parameter ranges

### Adding Features

Want to extend Skald? Some ideas:

- Multiple MIDI channels per dot (color-coded)
- Euclidean rhythm generator
- MIDI CC modulation per dot
- Multiple concurrent turntables
- Export patterns as MIDI files
- Preset browser

---

## Cross-Platform Building

### Windows
```bash
# Requires Visual Studio 2019 or later
cmake -B build -G "Visual Studio 16 2019"
cmake --build build --config Release
```

### Linux
```bash
# Requires build essentials
sudo apt-get install build-essential libasound2-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libfreetype6-dev
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

---

## License

This project is licensed under **GPL-3.0** due to JUCE framework licensing.

### JUCE Licensing
- **Open Source (GPL)**: Free to use and distribute under GPL-3.0
- **Commercial**: Requires JUCE commercial license (~$40/month) for closed-source distribution

See [JUCE Licensing](https://juce.com/juce-licensing) for details.

---

## Credits

**Skald** is built by **Beowulf Audio**

### Inspiration
- **Quintron's Drum Buddy** - Mechanical light-based drum machine
- **Playtonica MIDI Color Sequencer Orbita** - Visual color sequencer

### Built With
- [JUCE Framework](https://juce.com) - Audio plugin framework
- [CMake](https://cmake.org) - Build system
- Viking graphics by Beowulf Audio

---

## Support & Community

### Documentation
- [Quick Start Guide](QUICK_START.md) - Get started in minutes
- [In-app Help](QUICK_START.md#interface-overview) - Press HELP button in plugin

### Troubleshooting
- Plugin not showing? Rescan plugins in your DAW
- No sound? Check MIDI routing (Skald doesn't make sound, it generates MIDI)
- Notes not triggering? Check Probability knob (set to 100% for testing)

---

## Changelog

### v1.0.0 (2025-01-14)
- Initial release
- 13 musical scales with chromatic key selection
- ±2 octave range
- Probability, swing, and velocity variation
- Motor control and reverse playback
- Scratching support
- Save/Load pattern functionality
- Randomize pattern generator
- Clean, professional UI with Viking warrior branding

---

## Roadmap

Future enhancements being considered:

- [ ] Multiple MIDI channels (color-coded dots)
- [ ] Per-dot velocity and gate time
- [ ] Euclidean rhythm mode
- [ ] MIDI CC output for modulation
- [ ] Pattern preset browser
- [ ] MIDI file export
- [ ] Multiple turntables
- [ ] Tempo-independent mode

---

**Built with ⚔️ by Beowulf Audio**

*Mechanical rhythm meets modern production.*

🎵 **Make something unique!** 🎵
