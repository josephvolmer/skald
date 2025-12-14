# Makefile for Skald (Viking MIDI Warrior) VST Plugin
# Cross-platform build system using CMake

# Build configuration
BUILD_TYPE ?= Release
BUILD_DIR ?= build
JUCE_DIR ?= ../JUCE

# Number of parallel jobs
JOBS ?= 4

# Platform detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    PLATFORM := macOS
    VST3_INSTALL := ~/Library/Audio/Plug-Ins/VST3
    AU_INSTALL := ~/Library/Audio/Plug-Ins/Components
else ifeq ($(UNAME_S),Linux)
    PLATFORM := Linux
    VST3_INSTALL := ~/.vst3
else
    PLATFORM := Windows
    VST3_INSTALL := C:/Program Files/Common Files/VST3
endif

.PHONY: all clean configure build install test help

# Default target
all: build

# Help target
help:
	@echo "Skald VST Plugin - Build System"
	@echo "================================"
	@echo ""
	@echo "Available targets:"
	@echo "  make all        - Configure and build (default)"
	@echo "  make configure  - Run CMake configuration"
	@echo "  make build      - Build the plugin"
	@echo "  make clean      - Remove build directory"
	@echo "  make install    - Install plugin to system"
	@echo "  make rebuild    - Clean and rebuild"
	@echo "  make help       - Show this help message"
	@echo ""
	@echo "Build options:"
	@echo "  BUILD_TYPE=Debug|Release  - Set build type (default: Release)"
	@echo "  JOBS=N                    - Number of parallel jobs (default: 4)"
	@echo ""
	@echo "Examples:"
	@echo "  make BUILD_TYPE=Debug     - Build debug version"
	@echo "  make JOBS=8               - Build with 8 parallel jobs"
	@echo "  make clean build          - Clean rebuild"
	@echo ""
	@echo "Platform: $(PLATFORM)"
	@echo "Build directory: $(BUILD_DIR)"

# Configure CMake
configure:
	@echo "Configuring Skald for $(PLATFORM) ($(BUILD_TYPE))..."
	cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

# Build the plugin
build: configure
	@echo "Building Skald..."
	cmake --build $(BUILD_DIR) --config $(BUILD_TYPE) -j$(JOBS)
	@echo ""
	@echo "Build complete!"
	@echo "VST3: $(BUILD_DIR)/Skald_artefacts/$(BUILD_TYPE)/VST3/Skald.vst3"
ifeq ($(PLATFORM),macOS)
	@echo "AU:   $(BUILD_DIR)/Skald_artefacts/$(BUILD_TYPE)/AU/Skald.component"
endif

# Clean build directory
clean:
	@echo "Cleaning build directory..."
	rm -rf $(BUILD_DIR)
	@echo "Clean complete!"

# Rebuild (clean + build)
rebuild: clean build

# Install to system plugin directories
install: build
	@echo "Installing Skald to system directories..."
ifeq ($(PLATFORM),macOS)
	@echo "Installing VST3..."
	mkdir -p $(VST3_INSTALL)
	cp -R $(BUILD_DIR)/Skald_artefacts/$(BUILD_TYPE)/VST3/Skald.vst3 $(VST3_INSTALL)/
	@echo "Installing AU..."
	mkdir -p $(AU_INSTALL)
	cp -R $(BUILD_DIR)/Skald_artefacts/$(BUILD_TYPE)/AU/Skald.component $(AU_INSTALL)/
	@echo "Installation complete!"
	@echo "VST3 installed to: $(VST3_INSTALL)/Skald.vst3"
	@echo "AU installed to:   $(AU_INSTALL)/Skald.component"
else ifeq ($(PLATFORM),Linux)
	@echo "Installing VST3..."
	mkdir -p $(VST3_INSTALL)
	cp -R $(BUILD_DIR)/Skald_artefacts/$(BUILD_TYPE)/VST3/Skald.vst3 $(VST3_INSTALL)/
	@echo "Installation complete!"
	@echo "VST3 installed to: $(VST3_INSTALL)/Skald.vst3"
else
	@echo "Windows installation requires manual copying"
	@echo "VST3 location: $(BUILD_DIR)/Skald_artefacts/$(BUILD_TYPE)/VST3/Skald.vst3"
endif

# Test build (just verify it compiles)
test: build
	@echo "Build test passed!"
